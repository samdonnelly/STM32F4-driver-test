/**
 * @file gps_nav_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS navigation testing implementation 
 * 
 * @details The purpose of this code is to test navigation between predefined waypoints 
 *          using a compass (magnetometer) and a GNSS module. Once the GNSS device has a 
 *          position lock, the code reads the current location and compares it against 
 *          the first target location/waypoint by finding the distance and initial heading 
 *          between the two points. The compass heading is read which is then compared 
 *          to the heading between the two GNSS locations to find a heading error. The 
 *          heading error will indicate how much and in what direction the compass needs 
 *          to turn to be pointing at the target. The distance will indicate how close 
 *          the GNSS device is to the target. Using this, the target location can be 
 *          navigated to. Once the GNSS device has come within a certain radius of the 
 *          target, then the target will update to the next predefined waypoint. This 
 *          information will be output to a serial terminal for the user to see. 
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

// Conditional compilation 
#define GPS_NAV_TEST_SCREEN_ON_BUS 1    // HD44780U screen on same I2C bus as device 

// Configuration 
#define COORDINATE_LPF_GAIN 0.5   // Coordinate low pass filter gain 
#define HEADING_LPF_GAIN 0.2      // Heading low pass filter gain 
#define TN_OFFSET 130             // Offset between magnetic and true north (degrees*10) 
#define COORDINATE_RADIUS 100     // Threshold distance to target (meters*10) 

// Timing 
#define SAMPLE_INTERVAL 100000    // Interval between data reads/checks (us) 
#define GNSS_SAMPLE_COUNTER 10    // Number of intervals to elapse before checking the GPS 

// Data output 
#define OUTPUT_LENGTH 70          // Max data string output length 

//=======================================================================================


//=======================================================================================
// Classes 

class gps_nav_test : public nav_calculations 
{
private:   // Private variables 

    // GNSS 
    gps_waypoints_t current;           // Current location coordinates 
    gps_waypoints_t target;            // Desired waypoint coordinates 
    uint8_t waypoint_index;            // Index of target waypoints 
    int32_t radius;                    // Distance between current and desired location 
    uint8_t navstat;                   // Position lock status 
    
    // Heading 
    int16_t coordinate_heading;        // Heading between current and desired location 
    int16_t compass_heading;           // Current compass heading 
    int16_t error_heading;             // Error between compass and coordinate heading 

    // Timer information 
    TIM_TypeDef *timer_nonblocking;    // Timer used for non-blocking delays 
    tim_compare_t data_timer;          // Data sampling delay timing info 
    uint8_t timer_counter;             // GPS data update counter/timer 

    // Status 
    M8Q_STATUS m8q_status; 
    LSM303AGR_STATUS lsm303agr_status; 

public:   // Setup and teardown 
    
    // Constructor 
    gps_nav_test(
        TIM_TypeDef *timer, 
        double coordinate_filter_gain, 
        int16_t tn_offset) 
        : waypoint_index(CLEAR), 
          radius(COORDINATE_RADIUS), 
          navstat(CLEAR), 
          coordinate_heading(CLEAR), 
          compass_heading(CLEAR), 
          error_heading(CLEAR), 
          timer_nonblocking(timer), 
          timer_counter(CLEAR), 
          m8q_status(M8Q_OK), 
          lsm303agr_status(LSM303AGR_OK) 
    {
        // GNSS 
        current.lat = CLEAR; 
        current.lon = CLEAR; 
        target.lat = waypoints_0[waypoint_index].lat; 
        target.lon = waypoints_0[waypoint_index].lon; 

        // Navigation calculations 
        set_coordinate_lpf_gain(coordinate_filter_gain); 
        set_tn_offset(tn_offset); 
    }

    // Destructor 
    ~gps_nav_test() {}

public:   // Navigation 

    /**
     * @brief Perform GPS navigation 
     * 
     * @details Updates device and navigation data at an interval and outputs the results 
     *          to the serial terminal for the user to see. This allows the user to 
     *          navigate between GPS waypoints. 
     */
    void gps_navigation(void); 


    /**
     * @brief Configure the non-blocking timing information 
     * 
     * @details The non-blocking timer frequency can only be read after the timer has been 
     *          set up. That means this function must be called independently and setup 
     *          can't be handled in the constructor. 
     */
    void non_blocking_timer_config(void); 

private:   // Private members 
    
    /**
     * @brief Determine the needed heading 
     * 
     * @details Gets the current heading of the compass/magnetometer, corrects it to 
     *          be the true north heading, then finds the error between this heading 
     *          and the desired/target heading. The desired heading is the heading 
     *          between the current and target GPS locations. 
     */
    void nav_heading(void); 

    /**
     * @brief Evaluate the location 
     * 
     * @details Gets the updated GPS location, finds the distance and heading between 
     *          the current and target locations, and checks if the current location 
     *          has come within a certain distance of the tagret waypoint. If the 
     *          current location is close enough to the target, the target gets updated 
     *          to the next target waypoint. 
     */
    void nav_location(void); 

    /**
     * @brief Output the navigation results 
     */
    void nav_info_output(void); 

    /**
     * @brief Check for device driver faults 
     */
    void nav_status_check(void); 
}; 


// GPS navigation instance 
static gps_nav_test gps_nav(TIM9, COORDINATE_LPF_GAIN, TN_OFFSET); 

//=======================================================================================


//=======================================================================================
// Prototypes 

// M8Q initialization 
void gps_nav_test_m8q_init(void); 


// LSM303AGR initialization 
void gps_nav_test_lsm303agr_init(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void gps_nav_test_init(void)
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

    // HD44780U screen initialization 
#if GPS_NAV_TEST_SCREEN_ON_BUS 
    // If the HD44780U screen is on the same I2C bus as the LSM303AGR then the screen 
    // must be set up first to prevent it from interfering with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_display_off(); 
    hd44780u_backlight_off(); 
#endif   // GPS_NAV_TEST_SCREEN_ON_BUS 

    // M8Q setup 
    gps_nav_test_m8q_init(); 

    // LSM303AGR magnetometer setup  
    gps_nav_test_lsm303agr_init(); 

    // Configure the non-blocking timer 
    gps_nav.non_blocking_timer_config(); 
}


// M8Q initialization 
void gps_nav_test_m8q_init(void)
{
    // M8Q driver setup 
    M8Q_STATUS m8q_init_check = m8q_init(
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
    if (m8q_init_check || low_pwr_init_check || txr_init_check)
    {
        uart_sendstring(USART2, "\r\nM8Q init status: "); 
        uart_send_integer(USART2, (int16_t)m8q_init_check); 
        uart_sendstring(USART2, "\r\nM8Q low power pin init status: "); 
        uart_send_integer(USART2, (int16_t)low_pwr_init_check); 
        uart_sendstring(USART2, "\r\nM8Q TX ready pin init status: "); 
        uart_send_integer(USART2, (int16_t)txr_init_check); 

        while (TRUE); 
    }

    // M8Q controller setup 
    m8q_controller_init(TIM9); 
}


// LSM303AGR initialization 
void gps_nav_test_lsm303agr_init(void)
{
    // LSM303AGR magnetometer driver setup  
    LSM303AGR_STATUS lsm303agr_init_check = lsm303agr_m_init(
        I2C1, 
        lsm303agr_config_dir_offsets_0, 
        HEADING_LPF_GAIN, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 

    if (lsm303agr_init_check)
    {
        uart_sendstring(USART2, "\r\nLSM303AGR init status: "); 
        uart_send_integer(USART2, (int16_t)lsm303agr_init_check); 
        while (TRUE); 
    }
}


// Configure the non-blocking timing information 
void gps_nav_test::non_blocking_timer_config(void)
{
    data_timer.clk_freq = tim_get_pclk_freq(timer_nonblocking); 
    data_timer.time_cnt_total = CLEAR; 
    data_timer.time_cnt = CLEAR; 
    data_timer.time_start = SET_BIT; 
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
    
// Perform GPS navigation 
void gps_nav_test::gps_navigation(void)
{
    // Update the heading and GPS data at an interval 
    if (tim_compare(timer_nonblocking, 
                    data_timer.clk_freq, 
                    SAMPLE_INTERVAL, 
                    &data_timer.time_cnt_total, 
                    &data_timer.time_cnt, 
                    &data_timer.time_start))
    {
        // Update the heading 
        nav_heading(); 

        // Update the GPS information and user navigation info 
        if (timer_counter++ >= GNSS_SAMPLE_COUNTER)
        {
            timer_counter = CLEAR; 
            nav_location(); 
            nav_info_output(); 
        }

        // Check driver status 
        nav_status_check(); 
    }
}


// Determine the needed heading 
void gps_nav_test::nav_heading(void)
{
    // Update the compass heading, determine the true north heading and find the 
    // error between the current (compass) and desired (GPS) headings. Heading error 
    // is determined here and not with each location update so it's updated faster. 
    lsm303agr_status = lsm303agr_m_update(); 
    compass_heading = true_north_heading(lsm303agr_m_get_heading()); 
    error_heading = heading_error(compass_heading, coordinate_heading); 
}


// Evaluate the location 
void gps_nav_test::nav_location(void)
{
    gps_waypoints_t device_coordinates; 
    navstat = m8q_get_position_navstat_lock(); 

    if (navstat)
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
            // Adjust waypoint index 
            if (++waypoint_index >= NUM_GPS_WAYPOINTS_0)
            {
                waypoint_index = CLEAR; 
            }

            // Update the target waypoint 
            target.lat = waypoints_0[waypoint_index].lat; 
            target.lon = waypoints_0[waypoint_index].lon; 
        }
    }
    else 
    {
        // No position lock 
        radius = CLEAR; 
        error_heading = CLEAR; 
    }
}


// Output the navigation results 
void gps_nav_test::nav_info_output(void)
{
    char output_buff[OUTPUT_LENGTH]; 

    // Format the navigation data into a string 
    snprintf(
        output_buff, 
        OUTPUT_LENGTH, 
        "NAVSTAT: %u\r\nRadius: %ld     \r\nHeading Error: %d     \r\n", 
        navstat, radius, error_heading); 

    // Overwrite the old navigation data 
    uart_sendstring(USART2, "\033[1A\033[1A\033[1A"); 
    uart_sendstring(USART2, output_buff); 
}


// Check for device driver faults 
void gps_nav_test::nav_status_check(void)
{
    if ((m8q_get_state() == M8Q_FAULT_STATE) || lsm303agr_status)
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "\r\nM8Q state: "); 
        uart_send_integer(USART2, (int16_t)m8q_get_state()); 
        uart_sendstring(USART2, "\r\nLSM303AGR status: "); 
        uart_send_integer(USART2, (int16_t)lsm303agr_status); 
        while (TRUE); 
    }
}

//=======================================================================================
