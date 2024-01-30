/**
 * @file gps_nav_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS navigation testing implementation 
 * 
 * @details The purpose of this code is to test the devices position against predefined 
 *          waypoints. This code reads the devices current location and calculates the 
 *          surface distance to a target waypoint. If the distance is less than a threshold 
 *          then the device is considered to have hit the waypoint at which point the next 
 *          waypoint is selected and the process repeats. The distance to the next waypoint 
 *          is displayed on a screen for user feedback. The is no indication of direction 
 *          to the next waypoint other that the current distance to it. The distance will 
 *          only be updated as often as the device sends new position data (once per second). 
 *          If there is no position lock by the device then the code will wait until there 
 *          is a lock before doing an waypoint calculations. 
 * 
 * @version 0.1
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "gps_nav_test.h" 
#include "m8q_config.h" 
#include "lsm303agr_config.h" 
#include "gps_coordinates.h" 
#include "includes_cpp_drivers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define SAMPLE_INTERVAL 100000   // Interval between data reads/checks (us) 
#define GPS_SAMPLE_COUNTER 10    // Number of intervals to elapse before checking the GPS 
#define COORDINATE_RADIUS 100    // Threshold distance to target (meters*10) 

//=======================================================================================


//=======================================================================================
// Classes 

class gps_nav_test : public nav_calculations 
{
private:   // Private variables 

    // GPS 
    gps_waypoints_t current; 
    gps_waypoints_t target; 
    uint8_t waypoint_index; 
    int32_t radius; 
    
    // Heading 
    int16_t coordinate_heading; 
    int16_t compass_heading; 
    int16_t error_heading; 

    // Timer information 
    TIM_TypeDef *timer_nonblocking;        // Timer used for non-blocking delays 
    tim_compare_t data_timer;              // Data sampling delay timing info 
    uint8_t timer_counter; 

public:   // Setup and teardown 
    
    // Constructor 
    gps_nav_test(); 

    // Destructor 
    ~gps_nav_test(); 

public:   // Navigation 

    // Perform GPS navigation 
    void gps_navigation(void); 

private: 
    
    // Determine the needed heading 
    void nav_heading(void); 

    // Evaluate the location 
    void nav_location(void); 
}; 


static gps_nav_test gps_nav; 

//=======================================================================================


//=======================================================================================
// Global variables 
//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup code 

void gps_nav_test_init(void)
{
    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Initialize I2C
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    //==================================================

    //==================================================
    // Conditional setup 

#if GPS_NAV_TEST_SCREEN_ON_BUS 
    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering  with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_backlight_off(); 
#endif   // GPS_NAV_TEST_SCREEN_ON_BUS 
    
    //==================================================

    //==================================================
    // M8Q GPS setup 

    // Driver setup 
    M8Q_STATUS init_check = m8q_init(
        I2C1, 
        &m8q_config_pkt_0[0][0], 
        M8Q_CONFIG_NUM_MSG_PKT_0, 
        M8Q_CONFIG_MAX_LEN_PKT_0, 
        CLEAR); 

    // Set up low power and TX ready pins 
    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init(GPIOC, PIN_10); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init(GPIOC, PIN_11); 

    // Check if there was a problem during device initialization. If so, output the faults 
    // to the serial terminal and halt to program. 
    if (init_check || low_pwr_init_check || txr_init_check)
    {
        uart_sendstring(USART2, "\r\nDevice init status: "); 
        uart_send_integer(USART2, (int16_t)init_check); 
        uart_sendstring(USART2, "\r\nLow power pin init status: "); 
        uart_send_integer(USART2, (int16_t)low_pwr_init_check); 
        uart_sendstring(USART2, "\r\nTX Ready pin init status: "); 
        uart_send_integer(USART2, (int16_t)txr_init_check); 

        while (TRUE); 
    }

    // Controller setup 
    m8q_controller_init(TIM9); 
    
    //==================================================

    //==================================================
    // LSM303AGR Magnetometer (compasss) setup  

    // Driver setup 
    lsm303agr_init(
        I2C1, 
        lsm303agr_config_dir_offsets, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void gps_nav_test_app(void)
{
    m8q_controller(); 
    gps_nav.gps_navigation(); 
}

//=======================================================================================


//=======================================================================================
// Test functions 
    
// Constructor 
gps_nav_test::gps_nav_test() {}


// Destructor 
gps_nav_test::~gps_nav_test() {}


// Perform GPS navigation 
void gps_nav_test::gps_navigation(void)
{
    // Update the heading at an interval 
    if (tim_compare(timer_nonblocking, 
                    data_timer.clk_freq, 
                    SAMPLE_INTERVAL, // 0.1s 
                    &data_timer.time_cnt_total, 
                    &data_timer.time_cnt, 
                    &data_timer.time_start))
    {
        // Update the heading 
        nav_heading(); 

        // Update the GPS information 
        if (timer_counter++ >= GPS_SAMPLE_COUNTER)
        {
            timer_counter = CLEAR; 
            nav_location(); 
        }
    }
}


// Determine the needed heading 
void gps_nav_test::nav_heading(void)
{
    // Update the compass heading, determine the true north heading and find the 
    // error between the current (compass) and desired (GPS) headings. 
    lsm303agr_m_read(); 
    compass_heading = true_north_heading(lsm303agr_m_get_heading()); 
    error_heading = heading_error(compass_heading, coordinate_heading); 
}


// Evaluate the location 
void gps_nav_test::nav_location(void)
{
    gps_waypoints_t device_coordinates; 

    if (m8q_get_position_navstat_lock())
    {
        // Get the updated location by reading the GPS device coordinates then filtering 
        // the result. 
        device_coordinates.lat = m8q_get_position_lat(); 
        device_coordinates.lon = m8q_get_position_lon(); 
        coordinate_filter(device_coordinates, current); 

        // Calculate the distance to the target location and the heading needed to get 
        // there. 
        radius = gps_radius(current, target); 
        coordinate_heading = gps_heading(current, target); 

        // Check if the distance to the target is within the threshold. If so, the 
        // target is considered "hit" and we can move to the next target. 
        if (radius < COORDINATE_RADIUS)
        {
            // Update the target waypoint 
            target.lat = waypoints_0[waypoint_index].lat; 
            target.lon = waypoints_0[waypoint_index].lon; 

            // Adjust waypoint index 
            if (++waypoint_index == NUM_GPS_WAYPOINTS_0)
            {
                waypoint_index = CLEAR; 
            }
        }
    }
    else 
    {
        // No position lock. 
    }
}

//=======================================================================================














//=======================================================================================
// Old code 

//==================================================
// Macros 

// Location testing 
#define GPS_NAV_TEST_CALC_SCALE 10       // Scalar for calculated data 
#define GPS_NAV_TEST_PI_RAD 3.14159      // PI 
#define GPS_NAV_TEST_180_DEG 180         // 180 degrees 
#define GPS_NAV_TEST_EARTH_RAD 6371      // Earch average radius (km) 
#define GPS_NAV_TEST_KM_TO_M 1000        // Km to m conversion 
#define GPS_NAV_TEST_HEADING_GAIN 0.5    // GPS heading low pass filter gain 
#define GPS_NAV_TEST_RADIUS_GAIN 0.5     // GPS radius low pass filter gain 

// Navigation test 
#define GPS_NAV_TEST_CALC_SCALE 10      // Scalar for calculated data 
#define GPS_NAV_TEST_PI_RAD 3.14159     // PI 
#define GPS_NAV_TEST_180_DEG 180        // 180 degrees 
#define GPS_NAV_TEST_EARTH_RAD 6371     // Earch average radius (km) 
#define GPS_NAV_TEST_KM_TO_M 1000       // Km to m conversion 
#define GPS_NAV_TEST_HEADING_GAIN 0.5   // GPS heading low pass filter gain 
#define GPS_NAV_TEST_RADIUS_GAIN 0.5    // GPS radius low pass filter gain 
#define GPS_NAV_TEST_NUM_DIRS 8         // Number of directions of heading offset calcs 
#define GPS_NAV_TEST_M_READ_INT 100000  // Magnetometer read interval (us) 
#define GPS_NAV_TEST_GPS_RAD 50         // GPS radius threshold (m*10) 
#define GPS_NAV_TEST_KP 17              // Proportional control constant 
#define GPS_NAV_TEST_KI 0               // Integral control constant 
#define GPS_NAV_TEST_KD 0               // Derivative control constant 
#define GPS_NAV_TEST_PWM_N 30000        // ESC/motor neutral PWM count 
#define GPS_NAV_TEST_PWM_BASE 3000      // ESC/motor base speed PWM count 

//==================================================


//==================================================
// Variables 

// Target waypoint 
static gps_waypoints_t target_waypoint; 

// Screen message buffer 
static char screen_msg[20]; 


// True North correction 
// This is used to correct the calculated heading to point true North. True North and 
// Magnetic North are offset and the heading difference between them will change 
// depending on where you are on Earth. GPS gets position based on true North so in 
// order to compare GPS heading with magnetometer heading, the magnetic North heading 
// must be compensated. 
// To obtain this value, a smart phone was used to find the difference between true and 
// magnetic North. 
// If this device was used over a large geographic area then it would be necessary to 
// update this correction factor dynamically. However, this device (as of now) is used 
// for relatively localized applications which means it works to have it set manually 
// by the user. This parameters is ideally stored by an application program somehow 
// (such as an SD card) so it can be read and overwritten as needed as opposed to 
// having it hard coded (like it is here). 
// The correction implemented in this code does not account for extreme cases such as 
// navigation in between teu and magnetic North locations. 
static const int16_t mag_tn_correction = 130;   // 13 degrees 


// Navigation test data record 
typedef struct gps_nav_test_nav_s 
{
    // Target waypoint 
    gps_waypoints_t waypoint; 

    // Screen message buffer 
    char screen_msg[HD44780U_LINE_LEN]; 

    // Timing information 
    TIM_TypeDef *timer_nonblocking;        // Timer used for non-blocking delays 
    tim_compare_t delay_timer;             // Delay timing info 

    // Calculation info 
    int16_t heading_error; 
    int16_t gps_heading; 
    int16_t mag_heading; 
    uint8_t waypoint_index; 
    uint8_t waypoint_status; 

    // PID controller 
    int16_t kp;                            // Proportional control constant 
    int16_t ki;                            // Integral control constant 
    int16_t kd;                            // Derivative control constant 
    int16_t err_sum;                       // Sum of errors - for integral 
    int16_t err_prev;                      // Previous error - for derivative 

    // PWM output 
    int16_t pid_out;                       // PID controller output 
    uint16_t m1_pwm_cnt;                   // Motor 1 PWM counter transition 
    uint16_t m2_pwm_cnt;                   // Motor 2 PWM counter transition 
}
gps_nav_test_nav_t; 

// Data record instance 
static gps_nav_test_nav_t gps_nav_test_nav; 

//==================================================


//==================================================
// Prototypes 

/**
 * @brief M8Q location test setup code 
 */
void gps_nav_test_init_temp(void); 


/**
 * @brief M8Q location test code 
 */
void gps_nav_test_app_temp(void); 


/**
 * @brief Get the true North heading 
 * 
 * @details Reads the heading from the magnetometer and adds the true North heading offset 
 *          stored in 'mag_tn_correction' (global variable below). After the offset is 
 *          added the heading is checked to see if it is outside the acceptable heading 
 *          range (0-359.9 degrees) and if it is then it's corrected to be withing range 
 *          (ex. 365 degrees gets corrected to 5 degrees which is the same thing). 
 * 
 * @return int16_t : True North heading 
 */
int16_t gps_nav_test_heading_temp(void); 


/**
 * @brief Heading error - done every heading update (~10Hz) 
 * 
 * @details 
 * 
 * @param heading_desired 
 * @param heading_current 
 * @return int16_t 
 */
int16_t gps_nav_test_heading_error_temp(
    int16_t heading_desired, 
    int16_t heading_current); 


/**
 * @brief GPS coordinate radius check 
 * 
 * @details Calculates the surface distance between the devices current location and the 
 *          target waypoint. The distance is returned in meters*10 (meters = radius/10). 
 *          The central angle between the devices location and the waypoint is found and
 *          used along with the average Earth radius to calculate the surface distance. 
 * 
 * @param lat1 : current device latitude 
 * @param lon1 : current device longitude 
 * @param lat2 : target waypoint latitude 
 * @param lon2 : target waypoint longitude 
 * @return int16_t : scaled surface distance between location and waypoint 
 */
int16_t gps_nav_test_gps_rad_temp(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 


/**
 * @brief GPS heading calculation 
 * 
 * @details Calculates the heading between the current device location and the target 
 *          waypoint. The heading is an angle between 0-359.9 degrees from true North in 
 *          the clockwise direction. The return value is the heading expressed in 
 *          degrees*10. 
 * 
 * @param lat1 : current device latitude 
 * @param lon1 : current device longitude 
 * @param lat2 : target waypoint latitude 
 * @param lon2 : target waypoint longitude 
 * @return int16_t : scaled 0-360 degree true North heading 
 */
int16_t gps_nav_test_gps_heading_temp(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 


/**
 * @brief Motor controller 
 * 
 * @details 
 * 
 * @param error 
 * @return int16_t 
 */
int16_t gps_nav_test_pid_temp(
    int16_t error); 

//==================================================


//==================================================
// Setup 

// Old code 
void gps_nav_test_init_temp(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Initialize I2C
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

#if GPS_NAV_TEST_SCREEN_ON_BUS 
    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering  with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_backlight_off(); 
#endif   // GPS_NAV_TEST_SCREEN_ON_BUS 

    // M8Q device setup 
    M8Q_STATUS init_check = m8q_init(
        I2C1, 
        &m8q_config_pkt_0[0][0], 
        M8Q_CONFIG_NUM_MSG_PKT_0, 
        M8Q_CONFIG_MAX_LEN_PKT_0, 
        CLEAR); 

    // Set up low power and TX ready pins 
    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init(GPIOC, PIN_10); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init(GPIOC, PIN_11); 

    // Check if there was a problem during device initialization. If so, output the faults 
    // to the serial terminal and halt to program. 
    if (init_check || low_pwr_init_check || txr_init_check)
    {
        uart_sendstring(USART2, "\r\nDevice init status: "); 
        uart_send_integer(USART2, (int16_t)init_check); 
        uart_sendstring(USART2, "\r\nLow power pin init status: "); 
        uart_send_integer(USART2, (int16_t)low_pwr_init_check); 
        uart_sendstring(USART2, "\r\nTX Ready pin init status: "); 
        uart_send_integer(USART2, (int16_t)txr_init_check); 

        while (TRUE); 
    }

    // // Navigation data record initialization 
    // memset((void *)gps_nav_test_nav.screen_msg, CLEAR, sizeof(gps_nav_test_nav.screen_msg)); 
    // gps_nav_test_nav.timer_nonblocking = TIM9; 
    // gps_nav_test_nav.delay_timer.clk_freq = 
    //     tim_get_pclk_freq(gps_nav_test_nav.timer_nonblocking); 
    // gps_nav_test_nav.delay_timer.time_cnt_total = CLEAR; 
    // gps_nav_test_nav.delay_timer.time_cnt = CLEAR; 
    // gps_nav_test_nav.delay_timer.time_start = SET_BIT; 
    // gps_nav_test_nav.heading_error = CLEAR; 
    // gps_nav_test_nav.gps_heading = CLEAR; 
    // gps_nav_test_nav.mag_heading = CLEAR; 
    // gps_nav_test_nav.waypoint_index = CLEAR; 
    // gps_nav_test_nav.waypoint_status = SET_BIT; 
    // gps_nav_test_nav.kp = GPS_NAV_TEST_KP; 
    // gps_nav_test_nav.ki = GPS_NAV_TEST_KI; 
    // gps_nav_test_nav.kd = GPS_NAV_TEST_KD; 
    // gps_nav_test_nav.err_sum = CLEAR; 
    // gps_nav_test_nav.err_prev = CLEAR; 
    // gps_nav_test_nav.pid_out = CLEAR; 
    // gps_nav_test_nav.m1_pwm_cnt = GPS_NAV_TEST_PWM_N + GPS_NAV_TEST_PWM_BASE; 
    // gps_nav_test_nav.m2_pwm_cnt = GPS_NAV_TEST_PWM_N - GPS_NAV_TEST_PWM_BASE; 

    // // LSM303AGR init 
    // mag_offsets[0] = -160;     // N  (0/360deg) direction heading offset (degrees * 10) 
    // mag_offsets[1] = 32;       // NE (45deg) direction heading offset (degrees * 10) 
    // mag_offsets[2] = 215;      // E  (90deg) direction heading offset (degrees * 10) 
    // mag_offsets[3] = 385;      // SE (135deg) direction heading offset (degrees * 10) 
    // mag_offsets[4] = 435;      // S  (180deg) direction heading offset (degrees * 10) 
    // mag_offsets[5] = 20;       // SW (225deg) direction heading offset (degrees * 10) 
    // mag_offsets[6] = -450;     // W  (270deg) direction heading offset (degrees * 10) 
    // mag_offsets[7] = -365;     // NW (315deg) direction heading offset (degrees * 10) 

    // // Driver init 
    // lsm303agr_init(
    //     I2C1, 
    //     mag_offsets, 
    //     LSM303AGR_M_ODR_10, 
    //     LSM303AGR_M_MODE_CONT, 
    //     LSM303AGR_CFG_DISABLE, 
    //     LSM303AGR_CFG_DISABLE, 
    //     LSM303AGR_CFG_DISABLE, 
    //     LSM303AGR_CFG_DISABLE); 
}

//==================================================


//==================================================
// Test code 

void gps_nav_test_app_temp(void)
{
    // Local variables 
    uint8_t run = CLEAR; 
    double lat_current = CLEAR; 
    double lon_current = CLEAR; 
    int16_t radius = CLEAR; 
    int16_t heading = CLEAR; 
    static uint8_t waypoint_index = CLEAR; 
    static uint8_t waypoint_status = SET_BIT; 
    // uint8_t run = CLEAR; 
    // uint16_t navstat = CLEAR; 
    // int16_t radius = CLEAR; 
    // double lat_current = CLEAR; 
    // double lon_current = CLEAR; 

    // // Read magnetometer heading at an interval 
    // if (tim_compare(gps_nav_test_nav.timer_nonblocking, 
    //                 gps_nav_test_nav.delay_timer.clk_freq, 
    //                 GPS_NAV_TEST_M_READ_INT, // (us) 
    //                 &gps_nav_test_nav.delay_timer.time_cnt_total, 
    //                 &gps_nav_test_nav.delay_timer.time_cnt, 
    //                 &gps_nav_test_nav.delay_timer.time_start))
    // {
    //     // Update the magnetometer data 
    //     lsm303agr_m_read(); 

    //     // Get the true North heading from the magnetometer 
    //     gps_nav_test_nav.mag_heading = gps_nav_test_heading_temp(); 
        
    //     // Use the GPS heading and the magnetometer heading to get a heading error 
    //     gps_nav_test_nav.heading_error = 
    //         gps_nav_test_heading_error_temp(gps_nav_test_nav.gps_heading, 
    //                                      gps_nav_test_nav.mag_heading); 

    //     // Calculate the motor PWM output 
    //     gps_nav_test_nav.pid_out = 
    //         (gps_nav_test_pid_temp(gps_nav_test_nav.heading_error) >> SHIFT_5); 
    //     gps_nav_test_nav.m1_pwm_cnt = 
    //         GPS_NAV_TEST_PWM_N + GPS_NAV_TEST_PWM_BASE + gps_nav_test_nav.pid_out; 
    //     gps_nav_test_nav.m2_pwm_cnt = 
    //         GPS_NAV_TEST_PWM_N - GPS_NAV_TEST_PWM_BASE + gps_nav_test_nav.pid_out; 
    // }

    // Once there is data available on the device, read all messages. The device will 
    // update once per second. If there is data then the 'run' flag is set which 
    // triggers the rest of the test code. 
    while (m8q_get_tx_ready())
    {
        m8q_read_data(); 
        run++; 
    }

    // If the run flag is not zero then that means new data has been read from the 
    // device and the rest of the test code can be executed. 
    if (run)
    {
        // Check the position lock status. Relative position to waypoints can only be 
        // determined with a position lock so this is a requirement before doing 
        // any calculation. 
        if (m8q_get_position_navstat() == M8Q_NAVSTAT_G3)
        {
            // Position found. Proceed to determine the distance between the 
            // devices current location and the next waypoint. 

            // Get the updated location 
            lat_current = m8q_get_position_lat(); 
            lon_current = m8q_get_position_lon(); 

            //==================================================
            // Evaluate target 

            // If the device is close enough to a waypoint then the next waypoint in the 
            // mission is selected. 'waypoint_status' indicates when it's time to read 
            // the next waypoint. 
            if (waypoint_status)
            {
                // Update the target waypoint 
                target_waypoint.lat = waypoints_0[waypoint_index].lat; 
                target_waypoint.lon = waypoints_0[waypoint_index].lon; 

                // The status will be set again if the device hits (gets close enough to) 
                // the current target waypoint. 
                waypoint_status = CLEAR; 

                // Adjust waypoint index. If the end of the waypoint mission is reached 
                // then start over from the beginning. 
                if (++waypoint_index == NUM_GPS_WAYPOINTS_0)
                {
                    waypoint_index = CLEAR; 
                }
            }
            // if (gps_nav_test_nav.waypoint_status)
            // {
            //     // Update the target waypoint 
            //     gps_nav_test_nav.waypoint.lat = 
            //         waypoints[gps_nav_test_nav.waypoint_index].lat; 
            //     gps_nav_test_nav.waypoint.lon = 
            //         waypoints[gps_nav_test_nav.waypoint_index].lon; 

            //     // The status will be set again if the device hits (gets close enough to) 
            //     // the current target waypoint. 
            //     gps_nav_test_nav.waypoint_status = CLEAR; 

            //     // Adjust waypoint index. If the end of the waypoint mission is reached 
            //     // then start over from the beginning. 
            //     if (++gps_nav_test_nav.waypoint_index == GPS_NAV_NUM_WAYPOINTS)
            //     {
            //         gps_nav_test_nav.waypoint_index = CLEAR; 
            //     }
            // }
            
            //==================================================

            //==================================================
            // Update data 

            // Update GPS radius. This is the surface distance (arc distance) between 
            // the target waypoint and the current location. The radius is expressed 
            // in meters*10. 
            radius = gps_nav_test_gps_rad_temp(
                lat_current, 
                lon_current, 
                target_waypoint.lat, 
                target_waypoint.lon); 
            // radius = lsm303agr_test_gps_rad(
            //     lat_current, 
            //     lon_current, 
            //     gps_nav_test_nav.waypoint.lat, 
            //     gps_nav_test_nav.waypoint.lon); 

            // Update the heading between the current location and the waypoint. The 
            // heading will be an angle between 0-359.9 degrees from true North in the 
            // clockwise direction between the devices current location and the 
            // waypoint location. The heading is expressed as degrees*10. 
            heading = gps_nav_test_gps_heading_temp(
                lat_current, 
                lon_current, 
                target_waypoint.lat, 
                target_waypoint.lon); 
            // gps_nav_test_nav.gps_heading = lsm303agr_test_gps_heading(
            //     lat_current, 
            //     lon_current, 
            //     gps_nav_test_nav.waypoint.lat, 
            //     gps_nav_test_nav.waypoint.lon); 
            
            //==================================================

            //==================================================
            // Display the navigation info to the screen 

            snprintf(screen_msg, 20, "Radius: %um   ", radius); 
            hd44780u_line_set(HD44780U_L1, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            snprintf(screen_msg, 20, "Heading: %udeg", heading); 
            hd44780u_line_set(HD44780U_L2, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L2); 

            // 
            // snprintf(
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_LINE_LEN, 
            //     "RAD: %um   ", 
            //     radius); 
            // hd44780u_line_set(
            //     HD44780U_L1, 
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_CURSOR_NO_OFFSET); 

            // snprintf(
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_LINE_LEN, 
            //     "ERR: %ddeg    ", 
            //     gps_nav_test_nav.heading_error); 
            // hd44780u_line_set(
            //     HD44780U_L2, 
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_CURSOR_NO_OFFSET); 

            // // snprintf(
            // //     gps_nav_test_nav.screen_msg, 
            // //     HD44780U_LINE_LEN, 
            // //     "GPS: %udeg    ", 
            // //     gps_nav_test_nav.gps_heading); 
            // // hd44780u_line_set(
            // //     HD44780U_L3, 
            // //     gps_nav_test_nav.screen_msg, 
            // //     HD44780U_CURSOR_NO_OFFSET); 

            // // snprintf(
            // //     gps_nav_test_nav.screen_msg, 
            // //     HD44780U_LINE_LEN, 
            // //     "MAG: %udeg    ", 
            // //     gps_nav_test_nav.mag_heading); 
            // // hd44780u_line_set(
            // //     HD44780U_L4, 
            // //     gps_nav_test_nav.screen_msg, 
            // //     HD44780U_CURSOR_NO_OFFSET); 
            
            // snprintf(
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_LINE_LEN, 
            //     "M1:  %u", 
            //     gps_nav_test_nav.m1_pwm_cnt); 
            // hd44780u_line_set(
            //     HD44780U_L3, 
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_CURSOR_NO_OFFSET); 

            // snprintf(
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_LINE_LEN, 
            //     "M2:  %u", 
            //     gps_nav_test_nav.m2_pwm_cnt); 
            // hd44780u_line_set(
            //     HD44780U_L4, 
            //     gps_nav_test_nav.screen_msg, 
            //     HD44780U_CURSOR_NO_OFFSET); 
            
            // hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            // hd44780u_send_line(HD44780U_L1); 
            // hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
            // hd44780u_send_line(HD44780U_L2); 
            // hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_NO_OFFSET);
            // hd44780u_send_line(HD44780U_L3); 
            // hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_NO_OFFSET);
            // hd44780u_send_line(HD44780U_L4); 
            
            //==================================================

            // Check the radius against a threshold. If the radius is less than the 
            // threshold then the waypoint is considered to be hit and the 
            // 'waypoint_status' can be set to indicate that the next waypoint should 
            // be used. The radius and threshold are expressed in meters * 10. 
            if (radius < 100)
            {
                // Indicate that a new waypoint needs to be read 
                waypoint_status = SET_BIT; 
                // gps_nav_test_nav.waypoint_status = SET_BIT; 
            }
        }
        else 
        {
            // No position lock. Display the status on the screen but do nothing else 
            // while there is no lock. 
            hd44780u_clear(); 
            // The below line gives a C++ string to char* warning. 
            // hd44780u_line_set(HD44780U_L1, "No connection", HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
        }
    }
}

//==================================================


//==================================================
// Test functions 

// Get the true North heading 
int16_t gps_nav_test_heading_temp(void)
{
    // Local variables 
    int16_t tn_heading = CLEAR; 

    // Get the magnetometer heading and add the true North correction 
    tn_heading = lsm303agr_m_get_heading() + mag_tn_correction; 

    // Adjust the true North heading if the corrected headed exceeds heading bounds 
    if (mag_tn_correction >= 0)
    {
        if (tn_heading >= LSM303AGR_M_HEAD_MAX)
        {
            tn_heading -= LSM303AGR_M_HEAD_MAX; 
        }
    }
    else 
    {
        if (tn_heading < 0)
        {
            tn_heading += LSM303AGR_M_HEAD_MAX; 
        }
    }

    return tn_heading; 
}


// Heading error 
int16_t gps_nav_test_heading_error_temp(
    int16_t heading_desired, 
    int16_t heading_current)
{
    // Local variables 
    int16_t heading_error = CLEAR; 

    // Calculate the heading error 
    heading_error = heading_desired - heading_current; 

    // Correct the error for when the heading crosses the 0/360 degree boundary 
    if (heading_error > LSM303AGR_M_HEAD_DIFF)
    {
        heading_error -= LSM303AGR_M_HEAD_MAX; 
    }
    else if (heading_error < -LSM303AGR_M_HEAD_DIFF)
    {
        heading_error += LSM303AGR_M_HEAD_MAX; 
    }

    return heading_error; 
}


// GPS coordinate radius check - calculate surface distance and compare to threshold 
int16_t gps_nav_test_gps_rad_temp(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t gps_rad = CLEAR; 
    static double surf_dist = CLEAR; 
    double eq1, eq2, eq3, eq4, eq5; 
    double deg_to_rad = GPS_NAV_TEST_PI_RAD/GPS_NAV_TEST_180_DEG; 
    double pi_over_2 = GPS_NAV_TEST_PI_RAD/2.0; 
    double earth_rad = GPS_NAV_TEST_EARTH_RAD; 
    double km_to_m = GPS_NAV_TEST_KM_TO_M; 

    // Convert coordinates to radians 
    lat1 *= deg_to_rad; 
    lon1 *= deg_to_rad; 
    lat2 *= deg_to_rad; 
    lon2 *= deg_to_rad; 

    eq1 = cos(pi_over_2 - lat2)*sin(lon2 - lon1); 
    eq2 = cos(pi_over_2 - lat1)*sin(pi_over_2 - lat2); 
    eq3 = sin(pi_over_2 - lat1)*cos(pi_over_2 - lat2)*cos(lon2 - lon1); 
    eq4 = sin(pi_over_2 - lat1)*sin(pi_over_2 - lat2); 
    eq5 = cos(pi_over_2 - lat1)*cos(pi_over_2 - lat2)*cos(lon2 - lon1); 

    // atan2 is used because it produces an angle between +/-180 (pi). The central angle 
    // should always be positive and never greater than 180. 
    // Calculate the radius using a low pass filter to smooth the data. 
    surf_dist += ((atan2(sqrt((eq2 - eq3)*(eq2 - eq3) + (eq1*eq1)), (eq4 + eq5)) * 
                 earth_rad*km_to_m) - surf_dist)*GPS_NAV_TEST_RADIUS_GAIN; 
    gps_rad = (int16_t)(surf_dist*GPS_NAV_TEST_CALC_SCALE); 

    return gps_rad; 
}


// GPS heading calculation 
int16_t gps_nav_test_gps_heading_temp(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t heading = CLEAR; 
    static double heading_temp = CLEAR; 
    double num, den; 
    double deg_to_rad = GPS_NAV_TEST_PI_RAD/GPS_NAV_TEST_180_DEG; 

    // Convert coordinates to radians 
    lat1 *= deg_to_rad; 
    lon1 *= deg_to_rad; 
    lat2 *= deg_to_rad; 
    lon2 *= deg_to_rad; 

    // Calculate the numerator and denominator of the atan calculation 
    num = cos(lat2)*sin(lon2-lon1); 
    den = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2-lon1); 

    // Calculate the heading between coordinates. 
    // A low pass filter is used to smooth the data. 
    heading_temp += (atan(num/den) - heading_temp)*GPS_NAV_TEST_HEADING_GAIN; 

    // Convert heading to degrees 
    heading = (int16_t)(heading_temp*GPS_NAV_TEST_CALC_SCALE/deg_to_rad); 

    // Correct the calculated heading if needed 
    if (den < 0)
    {
        heading += LSM303AGR_M_HEAD_DIFF; 
    }
    else if (num < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    return heading; 
}


// Motor controller - called after every heading update 
int16_t gps_nav_test_pid_temp(
    int16_t error)
{
    // Local variables 
    int16_t proportional = CLEAR; 
    int16_t integral = CLEAR; 
    int16_t derivative = CLEAR; 

    //==================================================
    // Proportional 

    // Calculate the proportional portion 
    proportional = GPS_NAV_TEST_KP*error; 
    
    //==================================================

    //==================================================
    // Integral 

    // Integrate the error 
    gps_nav_test_nav.err_sum += error; 

    // Cap the error if it is too large 
    if (gps_nav_test_nav.err_sum > 1800)
    {
        gps_nav_test_nav.err_sum = 1800; 
    }
    else if (gps_nav_test_nav.err_sum < -1800)
    {
        gps_nav_test_nav.err_sum = -1800; 
    }

    // Calculate the integral portion 
    integral = gps_nav_test_nav.err_sum * GPS_NAV_TEST_KI; 
    
    //==================================================

    //==================================================
    // Derivative 

    // Calculate the derivative portion 
    derivative = GPS_NAV_TEST_KP*(error - gps_nav_test_nav.err_prev); 
    gps_nav_test_nav.err_prev = error; 
    
    //==================================================

    // PID output 
    return proportional + integral + derivative; 
}

//==================================================

//=======================================================================================
