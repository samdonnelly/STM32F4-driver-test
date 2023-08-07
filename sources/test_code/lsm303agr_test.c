/**
 * @file lsm303agr_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_test.h" 

//=======================================================================================


//=======================================================================================
// TODO 
// - Test magnetometer heading with low power, idle (mode), different ODR, LFP enabled, 
//   and offset cancellation 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Get the true North heading 
 * 
 * @details 
 * 
 * @return int16_t 
 */
int16_t lsm303agr_test_heading(void); 


/**
 * @brief Heading error - done every heading update (~10Hz) 
 * 
 * @details 
 * 
 * @param heading_desired 
 * @param heading_current 
 * @return int16_t 
 */
int16_t lsm303agr_test_heading_error(
    int16_t heading_desired, 
    int16_t heading_current); 


/**
 * @brief Motor controller 
 * 
 * @details 
 * 
 * @param error 
 * @return int16_t 
 */
int16_t lsm303agr_test_pid(
    int16_t error); 

//=======================================================================================


//=======================================================================================
// Global variables 

// Magnetometer directional offsets to correct for heading errors 
static int16_t mag_offsets[LSM303AGR_TEST_NUM_DIRS]; 

#if LSM303AGR_TEST_AXIS 

static int16_t mx_data; 
static int16_t my_data; 
static int16_t mz_data; 

#endif   // LSM303AGR_TEST_AXIS 


#if LSM303AGR_TEST_NAV 

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

// Sample waypoints 
static m8q_test_waypoints_t waypoints[LSM303AGR_TEST_NUM_WAYPOINTS] = 
{
    {50.961980, -114.065980}, 
    {50.962400, -114.065990}, 
    {50.962180, -114.066330}, 
    {50.961760, -114.065950}, 
    {50.961920, -114.066250} 
}; 

// Navigation test data record 
typedef struct lsm303agr_test_nav_s 
{
    // Target waypoint 
    m8q_test_waypoints_t waypoint; 

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
}
lsm303agr_test_nav_t; 

// Data record instance 
static lsm303agr_test_nav_t lsm303agr_test_nav; 

#endif   // LSM303AGR_TEST_NAV 

//=======================================================================================


//=======================================================================================
// Setup code

void lsm303agr_test_init(void)
{
    // Setup code for the LSM303AGR here 

    //===================================================
    // Standard setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 

    // Initialize I2C1
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
    
    //===================================================

    //===================================================
    // Conditional setup 
    
#if LSM303AGR_TEST_SCREEN 
    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering  with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
#if !LSM303AGR_TEST_NAV 
    hd44780u_backlight_off(); 
#endif   // !LSM303AGR_TEST_NAV 
#endif   // LSM303AGR_TEST_SCREEN 

#if LSM303AGR_TEST_NAV 

    // M8Q device setup 
    char m8q_config_messages[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN]; 
    m8q_config_copy(m8q_config_messages); 

    // Driver init 
    m8q_init(
        I2C1, 
        GPIOC, 
        PIN_10, 
        PIN_11, 
        M8Q_CONFIG_MSG_NUM, 
        M8Q_CONFIG_MSG_MAX_LEN, 
        (uint8_t *)m8q_config_messages[0]); 

    // PWM init 

    // Navigation data record initialization 
    memset((void *)lsm303agr_test_nav.screen_msg, CLEAR, sizeof(lsm303agr_test_nav.screen_msg)); 
    lsm303agr_test_nav.timer_nonblocking = TIM9; 
    lsm303agr_test_nav.delay_timer.clk_freq = 
        tim_get_pclk_freq(lsm303agr_test_nav.timer_nonblocking); 
    lsm303agr_test_nav.delay_timer.time_cnt_total = CLEAR; 
    lsm303agr_test_nav.delay_timer.time_cnt = CLEAR; 
    lsm303agr_test_nav.delay_timer.time_start = SET_BIT; 
    lsm303agr_test_nav.heading_error = CLEAR; 
    lsm303agr_test_nav.gps_heading = CLEAR; 
    lsm303agr_test_nav.mag_heading = CLEAR; 
    lsm303agr_test_nav.waypoint_index = CLEAR; 
    lsm303agr_test_nav.waypoint_status = SET_BIT; 
    lsm303agr_test_nav.kp = LSM303AGR_TEST_KP; 
    lsm303agr_test_nav.ki = LSM303AGR_TEST_KI; 
    lsm303agr_test_nav.kd = LSM303AGR_TEST_KD; 

    //===================================================

#endif   // LSM303AGR_TEST_NAV 

    //==================================================
    // LSM303AGR init 

    // Set offsets. These are used to correct for errors in the magnetometer readings. This 
    // is application/device dependent so it is part of the device init and not integrated 
    // into the driver/library. For a given application/device, these values should not change 
    // so it is ok to have them hard coded into the application code. 
    // 
    // Calibration steps: 
    // 1. Set the below values to zero. 
    // 2. Make sure LSM303AGR_TEST_HEADING is enabled and build the project. 
    // 3. Connect the LSM303AGR to the STM32, connect the STM32F4 to your machine, open PuTTy 
    //    so you can see the output and flash the code. 
    // 4. Using a trusted compasss (such as the one on your phone), align it in the North 
    //    direction, then align the long edge of the LSM303AGR (X-axis) with the compass so 
    //    they're both pointing North. 
    // 5. Observe the output of the magnetometer via Putty (Note that depending on the compass 
    //    you use you may have to move it away from the magnetometer once it's aligned or else 
    //    else you could get magnetometer interference - this happens with phone compasses). 
    //    Note the difference between the magnetometer output and the compass heading and 
    //    record it in offsets[0] below. If the magnetometer reading is clockwise of the compass 
    //    heading then the value recorded will be negative. Recorded offsets are scaled by 10. 
    //    Compass and magnetometer heading are from 0-359 degrees. For example, if the compass 
    //    reads 0 degrees (Magnetic North) and the magnetometer output reads +105 (10.5 degrees) 
    //    then the offset recorded is -105. 
    // 6. Repeat steps 4 and 5 for all directions in 45 degree increments (NE, E, SE, etc.) and 
    //    record each subsequent direction in the next 'offsets' element. 

    mag_offsets[0] = 0;        // N  (0/360deg) direction heading offset (degrees * 10) 
    mag_offsets[1] = -250;     // NE (45deg) direction heading offset (degrees * 10) 
    mag_offsets[2] = -60;      // E  (90deg) direction heading offset (degrees * 10) 
    mag_offsets[3] = 50;       // SE (135deg) direction heading offset (degrees * 10) 
    mag_offsets[4] = 210;      // S  (180deg) direction heading offset (degrees * 10) 
    mag_offsets[5] = 335;      // SW (225deg) direction heading offset (degrees * 10) 
    mag_offsets[6] = 290;      // W  (270deg) direction heading offset (degrees * 10) 
    mag_offsets[7] = -310;     // NW (315deg) direction heading offset (degrees * 10) 

    // Driver init 
    lsm303agr_init(
        I2C1, 
        mag_offsets, 
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

void lsm303agr_test_app(void)
{
    // Test code for the LSM303AGR here 

#if LSM303AGR_TEST_HEADING 
    // Update the magnetometer data 
    lsm303agr_m_read(); 

#if LSM303AGR_TEST_AXIS 
    lsm303agr_m_get_data(&mx_data, &my_data, &mz_data); 

    // Display the first device results - values are scaled to remove decimal 
    uart_sendstring(USART2, "mx = ");
    uart_send_integer(USART2, mx_data);
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "my = ");
    uart_send_integer(USART2, my_data); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "mz = ");
    uart_send_integer(USART2, mz_data);
    uart_send_new_line(USART2); 
#endif   // LSM303AGR_TEST_AXIS 

    uart_sendstring(USART2, "heading = ");
    uart_send_integer(USART2, lsm303agr_m_get_heading());
    uart_send_spaces(USART2, UART_SPACE_2); 

    // Delay 
    tim_delay_ms(TIM9, 150);

#if LSM303AGR_TEST_AXIS 
    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 
#endif   // LSM303AGR_TEST_AXIS 

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 
#endif   // LSM303AGR_TEST_HEADING 

#if LSM303AGR_TEST_NAV 

    // Local variables 
    uint8_t run = CLEAR; 
    uint16_t navstat = CLEAR; 
    int16_t radius = CLEAR; 
    double lat_current = CLEAR; 
    double lon_current = CLEAR; 

    // Read magnetometer heading at an interval 
    // Wait for a short period of time before leaving the init state 
    if (tim_compare(lsm303agr_test_nav.timer_nonblocking, 
                    lsm303agr_test_nav.delay_timer.clk_freq, 
                    LSM303AGR_TEST_M_READ_INT, // (us) 
                    &lsm303agr_test_nav.delay_timer.time_cnt_total, 
                    &lsm303agr_test_nav.delay_timer.time_cnt, 
                    &lsm303agr_test_nav.delay_timer.time_start))
    {
        // Update the magnetometer data 
        lsm303agr_m_read(); 

        // Get the true North heading from the magnetometer 
        lsm303agr_test_nav.mag_heading = lsm303agr_test_heading(); 
        
        // Use the GPS heading and the magnetometer heading to get a heading error 
        lsm303agr_test_nav.heading_error = 
            lsm303agr_test_heading_error(lsm303agr_test_nav.gps_heading, 
                                         lsm303agr_test_nav.mag_heading); 
            
    }

    // Once there is data available on the device, read all messages. The device will 
    // update once per second. If there is data then the 'run' flag is set which 
    // triggers the rest of the test code. 
    while (m8q_get_tx_ready())
    {
        m8q_read(); 
        run++; 
    }

    // If the run flag is not zero then that means new data has been read from the 
    // device and the rest of the test code can be executed. 
    if (run)
    {
        // Check the position lock status. Relative position to waypoints can only be 
        // determined with a position lock so this is a requirement before doing 
        // any calculation. 
        navstat = m8q_get_navstat(); 
        if ((navstat == M8Q_NAVSTAT_G3) || (navstat == M8Q_NAVSTAT_G2))
        {
            // Position found. Proceed to determine the distance between the 
            // devices current location and the next waypoint. 

            // Get the updated location 
            lat_current = m8q_get_lat(); 
            lon_current = m8q_get_long(); 

            // If the device is close enough to a waypoint then the next waypoint in the 
            // mission is selected. 'waypoint_status' indicates when it's time to read 
            // the next waypoint. 
            if (lsm303agr_test_nav.waypoint_status)
            {
                // Update the target waypoint 
                lsm303agr_test_nav.waypoint.lat = 
                    waypoints[lsm303agr_test_nav.waypoint_index].lat; 
                lsm303agr_test_nav.waypoint.lon = 
                    waypoints[lsm303agr_test_nav.waypoint_index].lon; 

                // The status will be set again if the device hits (gets close enough to) 
                // the current target waypoint. 
                lsm303agr_test_nav.waypoint_status = CLEAR; 

                // Adjust waypoint index. If the end of the waypoint mission is reached 
                // then start over from the beginning. 
                if (++lsm303agr_test_nav.waypoint_index == LSM303AGR_TEST_NUM_WAYPOINTS)
                {
                    lsm303agr_test_nav.waypoint_index = CLEAR; 
                }
            }

            // Update GPS radius. This is the surface distance (arc distance) between 
            // the target waypoint and the current location. The radius us expressed 
            // in meters*10. 
            radius = m8q_test_gps_rad(
                lat_current, 
                lon_current, 
                lsm303agr_test_nav.waypoint.lat, 
                lsm303agr_test_nav.waypoint.lon); 

            // Update the heading between the current location and the waypoint. The 
            // heading will be an angle between 0-359.9 degrees from true North in the 
            // clockwise direction between the devices current location and the 
            // waypoint location. The heading is expressed as degrees*10. 
            lsm303agr_test_nav.gps_heading = m8q_test_gps_heading(
                lat_current, 
                lon_current, 
                lsm303agr_test_nav.waypoint.lat, 
                lsm303agr_test_nav.waypoint.lon); 

            // Display the navigation info to the screen 
            snprintf(
                lsm303agr_test_nav.screen_msg, 
                HD44780U_LINE_LEN, 
                "RAD: %um   ", 
                radius); 
            hd44780u_line_set(
                HD44780U_L1, 
                lsm303agr_test_nav.screen_msg, 
                HD44780U_CURSOR_NO_OFFSET); 

            snprintf(
                lsm303agr_test_nav.screen_msg, 
                HD44780U_LINE_LEN, 
                "GPS: %udeg    ", 
                lsm303agr_test_nav.gps_heading); 
            hd44780u_line_set(
                HD44780U_L2, 
                lsm303agr_test_nav.screen_msg, 
                HD44780U_CURSOR_NO_OFFSET); 

            snprintf(
                lsm303agr_test_nav.screen_msg, 
                HD44780U_LINE_LEN, 
                "MAG: %udeg    ", 
                lsm303agr_test_nav.mag_heading); 
            hd44780u_line_set(
                HD44780U_L3, 
                lsm303agr_test_nav.screen_msg, 
                HD44780U_CURSOR_NO_OFFSET); 

            snprintf(
                lsm303agr_test_nav.screen_msg, 
                HD44780U_LINE_LEN, 
                "ERR: %ddeg    ", 
                lsm303agr_test_nav.heading_error); 
            hd44780u_line_set(
                HD44780U_L4, 
                lsm303agr_test_nav.screen_msg, 
                HD44780U_CURSOR_NO_OFFSET); 
            
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L2); 
            hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L3); 
            hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L4); 

            // Check the radius against a threshold. If the radius is less than the 
            // threshold then the waypoint is considered to be hit and the 
            // 'waypoint_status' can be set to indicate that the next waypoint should 
            // be used. The radius and threshold are expressed in meters * 10. 
            if (radius < LSM303AGR_TEST_GPS_RAD)
            {
                // Indicate that a new waypoint needs to be read 
                lsm303agr_test_nav.waypoint_status = SET_BIT; 
            }
        }
        else 
        {
            // No position lock. Display the status on the screen but do nothing else 
            // while there is no lock. 
            hd44780u_clear(); 
            hd44780u_line_set(HD44780U_L1, "No connection", HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
        }
    }

#endif   // LSM303AGR_TEST_NAV 
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Get the true North heading 
int16_t lsm303agr_test_heading(void)
{
    // Local variables 
    int16_t tn_heading = CLEAR; 

    // Get the magnetometer and add the true North correction 
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
int16_t lsm303agr_test_heading_error(
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


// Motor controller - called after every heading update 
int16_t lsm303agr_test_pid(
    int16_t error)
{
    // Local variables 
    int16_t proportional = CLEAR; 
    int16_t integral = CLEAR; 
    int16_t derivative = CLEAR; 

    //==================================================
    // Proportional 

    // Calculate the proportional portion 
    proportional = LSM303AGR_TEST_KP*error; 
    
    //==================================================

    //==================================================
    // Integral 

    // Integrate the error 
    lsm303agr_test_nav.err_sum += error; 

    // Cap the error if it is too large 
    if (lsm303agr_test_nav.err_sum > 1800)
    {
        lsm303agr_test_nav.err_sum = 1800; 
    }
    else if (lsm303agr_test_nav.err_sum < -1800)
    {
        lsm303agr_test_nav.err_sum = -1800; 
    }

    // Calculate the integral portion 
    integral = lsm303agr_test_nav.err_sum * LSM303AGR_TEST_KI; 
    
    //==================================================

    //==================================================
    // Derivative 

    // Calculate the derivative portion 
    derivative = LSM303AGR_TEST_KP*(error - lsm303agr_test_nav.err_prev); 
    lsm303agr_test_nav.err_prev = error; 
    
    //==================================================

    // PID output 
    return proportional + integral + derivative; 
}

//=======================================================================================
