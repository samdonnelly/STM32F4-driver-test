/**
 * @file m8q_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_test.h"

//=======================================================================================


//=======================================================================================
// Global varaibles 

#if M8Q_CONTROLLER_TEST

// User command table 
static state_request_t m8q_state_cmds[M8Q_NUM_USER_CMDS] = 
{
    // Controller functions 
    {"rr_set",     SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"rr_clear",   SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"read_set",   SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"read_clear", SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"lp_set",     SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"lp_clear",   SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"reset",      SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0}, 
    {"state",      SMT_ARGS_0, SMT_STATE_FUNC_PTR_2, SMT_ARG_BUFF_POS_0}, 
    {"fault",      SMT_ARGS_0, SMT_STATE_FUNC_PTR_3, SMT_ARG_BUFF_POS_0}, 
    // Driver functions 
    {"navstat",    SMT_ARGS_0, SMT_STATE_FUNC_PTR_3, SMT_ARG_BUFF_POS_0}, 
    {"lat",        SMT_ARGS_0, SMT_STATE_FUNC_PTR_4, SMT_ARG_BUFF_POS_0}, 
    {"long",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_4, SMT_ARG_BUFF_POS_0}, 
    {"NS",         SMT_ARGS_0, SMT_STATE_FUNC_PTR_5, SMT_ARG_BUFF_POS_0}, 
    {"EW",         SMT_ARGS_0, SMT_STATE_FUNC_PTR_5, SMT_ARG_BUFF_POS_0}, 
    {"time",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_6, SMT_ARG_BUFF_POS_0}, 
    {"date",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_6, SMT_ARG_BUFF_POS_0}, 
    // Execute command 
    {"execute", 0, 0, 0} 
}; 


// Function pointer table 
static m8q_func_ptrs_t m8q_state_func[M8Q_NUM_USER_CMDS] = 
{
    // Controller functions 
    {&m8q_set_read_ready, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_clear_read_ready, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_set_read_flag, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_clear_read_flag, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_set_low_pwr_flag, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_clear_low_pwr_flag, NULL, NULL, NULL, NULL, NULL}, 
    {&m8q_set_reset_flag, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, &m8q_get_state, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, &m8q_get_fault_code, NULL, NULL, NULL}, 
    // Driver functions 
    {NULL, NULL, &m8q_get_navstat, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, &m8q_get_lat_str, NULL, NULL}, 
    {NULL, NULL, NULL, &m8q_get_long_str, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, &m8q_get_NS, NULL}, 
    {NULL, NULL, NULL, NULL, &m8q_get_EW, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, &m8q_get_time}, 
    {NULL, NULL, NULL, NULL, NULL, &m8q_get_date}, 
    // Execute command 
    {NULL, NULL, NULL, NULL, NULL, NULL} 
}; 

#endif   // M8Q_CONTROLLER_TEST 


#if M8Q_TEST_LOCATION 

// Sample waypoints 
static m8q_test_waypoints_t waypoints[M8Q_TEST_NUM_WAYPOINTS] = 
{
    {50.962010, -114.065890}, 
    {50.962340, -114.065930}, 
    {50.962340, -114.066260}, 
    {50.961730, -114.066080} 
}; 

// Target waypoint 
static m8q_test_waypoints_t waypoint; 

// Screen message buffer 
static char screen_msg[20]; 

#endif   // M8Q_TEST_LOCATION 

//=======================================================================================


//=======================================================================================
// Setup code

void m8q_test_init()
{
    //==================================================
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
    // Conditional Setup 
#if M8Q_USER_CONFIG 

    m8q_user_config_init(I2C1); 

#else   // M8Q_USER_CONFIG 

#if M8Q_CONTROLLER_TEST

    // Initialize the device controller 
    m8q_controller_init(TIM9); 

    // Initialize the state machine test code 
    state_machine_init(M8Q_NUM_USER_CMDS); 

#else   // M8Q_CONTROLLER_TEST 

#if M8Q_DATA_CHECK 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0x0032,  // ARR=50, (50 counts)*(100us/count) = 5ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM10); 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_0); 


    // User button setup. The user buttons are used to trigger data reads and 
    // data size checks. 

    // Initialize the GPIO pins for the buttons and the button debouncer 
    gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    gpio_pin_init(GPIOC, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    debounce_init(GPIOX_PIN_0 |GPIOX_PIN_1 ); 

#endif   // M8Q_DATA_CHECK 

#if M8Q_TEST_LOCATION 

    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering  with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 

#endif   // M8Q_TEST_LOCATION 

    // M8Q device setup 

    // Send the configuration messages to configure the device settings. The M8Q has 
    // no flash to store user settings. Instead they're saved RAM which can only be 
    // powered until the onboard backup battery loses power. For this reason, settings 
    // must always be configured in setup. 
    char m8q_config_messages[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_MSG_LEN]; 
    m8q_config_copy(m8q_config_messages); 

    // Driver init 
    m8q_init(
        I2C1, 
        GPIOC, 
        PIN_10, 
        PIN_11, 
        M8Q_CONFIG_NUM_MSG_PKT_0, 
        M8Q_CONFIG_MAX_MSG_LEN, 
        (uint8_t *)m8q_config_messages[0]); 
    
    // Output an initialization warning if a driver fault occurs on setup. 
    if (m8q_get_status()) 
    {
        uart_sendstring(USART2, "M8Q init fault.\r\n"); 
    }

#endif   // M8Q_CONTROLLER_TEST 

#endif   // M8Q_USER_CONFIG 

    // Delay to let everything finish setup before starting to send and receieve data 
    tim_delay_ms(TIM9, 500); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void m8q_test_app()
{
#if M8Q_USER_CONFIG 

    m8q_user_config(); 

#else 

#if M8Q_CONTROLLER_TEST 

    //==================================================
    // Controller test code 

    // Local variables 

    // General purpose arguments array 
    static char user_args[M8Q_MAX_SETTER_ARGS][STATE_USER_TEST_INPUT]; 

    // Control flags 
    uint8_t arg_convert = CLEAR; 
    uint32_t set_get_status = CLEAR; 
    uint8_t cmd_index = CLEAR; 

    // Data buffers 
    uint16_t navstat = CLEAR; 
    uint8_t deg_min[M8Q_COO_LEN]; 
    uint8_t min_frac[M8Q_COO_LEN]; 
    uint8_t utc[9]; 

    // Determine what to do from user input 
    state_machine_test(
        m8q_state_cmds, 
        user_args[0], 
        &cmd_index, 
        &arg_convert, 
        &set_get_status); 

    // Check if there are any setters or getters requested 
    if (set_get_status)
    {
        for (uint8_t i = 0; i < (M8Q_NUM_USER_CMDS-1); i++)
        {
            if ((set_get_status >> i) & SET_BIT)
            {
                switch (m8q_state_cmds[i].func_ptr_index)
                {
                    case SMT_STATE_FUNC_PTR_1: 
                        (m8q_state_func[i].setter_1)(); 
                        break; 

                    case SMT_STATE_FUNC_PTR_2: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(USART2, (int16_t)(m8q_state_func[i].getter_1)()); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_3: 
                        uart_send_new_line(USART2); 

                        if (i == 9)  // NAVSTAT index 
                        {
                            navstat = (m8q_state_func[i].getter_2)(); 
                            uart_sendchar(USART2, (uint8_t)(navstat >> SHIFT_8)); 
                            uart_sendchar(USART2, (uint8_t)(navstat)); 
                        }
                        else
                        {
                            uart_send_integer(USART2, (int16_t)(m8q_state_func[i].getter_2)()); 
                        }

                        uart_send_new_line(USART2); 
                        
                        break; 

                    case SMT_STATE_FUNC_PTR_4: 
                        (m8q_state_func[i].getter_3)(deg_min, min_frac); 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, "deg_min: "); 
                        uart_sendstring(USART2, (char *)deg_min); 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, "min_frac: "); 
                        uart_sendstring(USART2, (char *)min_frac); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_5: 
                        uart_send_new_line(USART2); 
                        uart_sendchar(USART2, (m8q_state_func[i].getter_4)()); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_6: 
                        memset((void *)utc, CLEAR, sizeof(utc)); 
                        (m8q_state_func[i].getter_5)(utc); 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, (char *)utc); 
                        uart_send_new_line(USART2); 
                        break; 

                    default: 
                        break; 
                }
            }
        }
    }

    // Call the device controller 
    m8q_controller(); 

    //==================================================

#else   // M8Q_CONTROLLER_TEST 

    //===================================================
    // Non-controller test code 
    
#if M8Q_MSG_COUNT 

    // Local variables 
    uint8_t count = CLEAR; 

    while (TRUE)
    {
        if (m8q_get_tx_ready())
        {
            m8q_read(); 
            count++; 
        }
        else
        {
            if (count)
            {
                uart_send_integer(USART2, (int16_t)count); 
                uart_send_new_line(USART2); 
            }
            break; 
        }
    }

#endif   // M8Q_MSG_COUNT 

#if M8Q_DATA_CHECK 

    // Test plan to check if data is skipped or overwritten: 
    // - Let the module get a connection and start reporting the UTC time 
    // - Coorelate the UTC time to local time as a reference 
    // - Read a time stamp and make note of it 
    // - Let the module data buffer fill up with data by checking the data size 
    // - Read data and check the updated time 
    // - If the time read is the most up to date then data should be getting over-
    //   written, otherwise data should be getting blocked. 

    // Answer to test question 
    // - Data is queued up in the devices message buffer and you must read the oldest 
    //   messages to clear the queue before you can read the newset messages. The device 
    //   message buffer is only so bit so once it is full then data is lost. 

    // Local variables 
    static uint8_t button_block_1 = CLEAR; 
    static uint8_t button_block_2 = CLEAR; 
    uint16_t data_size; 
    uint8_t current_time[10]; 

    //===================================================
    // Update the button status on periodic interrupt 

    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        // Clear interrupt handler 
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Update the user button status 
        debounce((uint8_t)gpio_port_read(GPIOC)); 
    }
    
    //===================================================

    //===================================================
    // Do things if the buttons are pressed 

    // If button 1 is pressed then read and check the time 
    if (debounce_pressed((uint8_t)GPIOX_PIN_0) && !button_block_1) 
    {
        if (m8q_get_tx_ready())
        {
            m8q_read(); 
            m8q_get_time(current_time); 
            uart_sendstring(USART2, (char *)current_time); 
            uart_send_new_line(USART2); 
            memset((void *)current_time, CLEAR, sizeof(current_time)); 
        }

        button_block_1 = SET_BIT; 
    }
    else if (debounce_released((uint8_t)GPIOX_PIN_0) && button_block_1) 
    {
        button_block_1 = CLEAR; 
    }

    // If button 2 is pressed then check the data size 
    if (debounce_pressed((uint8_t)GPIOX_PIN_1) && !button_block_2) 
    {
        if (m8q_get_tx_ready())
        {
            m8q_check_data_size(&data_size); 
            uart_send_integer(USART2, (int16_t)data_size); 
            uart_send_new_line(USART2); 
        }

        button_block_2 = SET_BIT; 
    }
    else if (debounce_released((uint8_t)GPIOX_PIN_1) && button_block_2) 
    {
        button_block_2 = CLEAR; 
    }

    //===================================================

#endif   // M8Q_DATA_CHECK 

#if M8Q_TEST_LOCATION 

    /**
     * @brief M8Q location test 
     * 
     * @details The purpose of this code is to test the devices position against 
     *          predefined coordinates or waypoints. 
     *          
     *          This code reads the devices current location and calculates the surface distance 
     *          to a target waypoint. If the distance is less than a threshold then the device is 
     *          considered to have hit the waypoint at which point the next waypoint is selected 
     *          and the process repeats. The distance to the next waypoint is displayed on a screen 
     *          for user feedback. The is no indication of direction to the next waypoint other 
     *          that the current distance to it. The distance will only be updated as often as 
     *          the device sends new position data (once per second). If there is no position 
     *          lock by the device then the code will wait until there is a lock before doing 
     *          an waypoint calculations.
     */

    // Local variables 
    uint8_t run = CLEAR; 
    double lat_current = CLEAR; 
    double lon_current = CLEAR; 
    int16_t radius = CLEAR; 
    int16_t heading = CLEAR; 
    static uint8_t waypoint_index = CLEAR; 
    static uint8_t waypoint_status = SET_BIT; 

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
        if (m8q_get_navstat() == M8Q_NAVSTAT_G3)
        {
            // Position found. Proceed to determine the distance between the 
            // devices current location and the next waypoint. 

            // Get the updated location 
            lat_current = m8q_get_lat(); 
            lon_current = m8q_get_long(); 

            // If the device is close enough to a waypoint then the next waypoint in the 
            // mission is selected. 'waypoint_status' indicates when it's time to read 
            // the next waypoint. 
            if (waypoint_status)
            {
                // Update the target waypoint 
                waypoint.lat = waypoints[waypoint_index].lat; 
                waypoint.lon = waypoints[waypoint_index].lon; 

                // The status will be set again if the device hits (gets close enough to) 
                // the current target waypoint. 
                waypoint_status = CLEAR; 

                // Adjust waypoint index. If the end of the waypoint mission is reached 
                // then start over from the beginning. 
                if (++waypoint_index == M8Q_TEST_NUM_WAYPOINTS)
                {
                    waypoint_index = CLEAR; 
                }
            }

            // Update GPS radius. This is the surface distance (arc distance) between 
            // the target waypoint and the current location. The radius is expressed 
            // in meters*10. 
            radius = m8q_test_gps_rad(
                lat_current, 
                lon_current, 
                waypoint.lat, 
                waypoint.lon); 

            // Update the heading between the current location and the waypoint. The 
            // heading will be an angle between 0-359.9 degrees from true North in the 
            // clockwise direction between the devices current location and the 
            // waypoint location. The heading is expressed as degrees*10. 
            heading = m8q_test_gps_heading(
                lat_current, 
                lon_current, 
                waypoint.lat, 
                waypoint.lon); 

            // Display the radius to the screen. 
            snprintf(screen_msg, 20, "Radius: %um   ", radius); 
            hd44780u_line_set(HD44780U_L1, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            snprintf(screen_msg, 20, "Heading: %udeg", heading); 
            hd44780u_line_set(HD44780U_L2, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L2); 

            // Check the radius against a threshold. If the radius is less than the 
            // threshold then the waypoint is considered to be hit and the 
            // 'waypoint_status' can be set to indicate that the next waypoint should 
            // be used. The radius and threshold are expressed in meters * 10. 
            if (radius < 100)
            {
                // Indicate that a new waypoint needs to be read 
                waypoint_status = SET_BIT; 
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

#endif   // M8Q_TEST_LOCATION 

    //===================================================

#endif   // M8Q_CONTROLLER_TEST

#endif   // M8Q_USER_CONFIG
}

//=======================================================================================


//=======================================================================================
// Test functions 

// GPS coordinate radius check - calculate surface distance and compare to threshold 
int16_t m8q_test_gps_rad(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t gps_rad = CLEAR; 
    static double surf_dist = CLEAR; 
    double eq1, eq2, eq3, eq4, eq5; 
    double deg_to_rad = M8Q_TEST_PI_RAD/M8Q_TEST_180_DEG; 
    double pi_over_2 = M8Q_TEST_PI_RAD/2.0; 
    double earth_rad = M8Q_TEST_EARTH_RAD; 
    double km_to_m = M8Q_TEST_KM_TO_M; 

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
                 earth_rad*km_to_m) - surf_dist)*M8Q_TEST_RADIUS_GAIN; 
    gps_rad = (int16_t)(surf_dist*M8Q_TEST_CALC_SCALE); 

    return gps_rad; 
}


// GPS heading calculation 
int16_t m8q_test_gps_heading(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t heading = CLEAR; 
    static double heading_temp = CLEAR; 
    double num, den; 
    double deg_to_rad = M8Q_TEST_PI_RAD/M8Q_TEST_180_DEG; 

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
    heading_temp += (atan(num/den) - heading_temp)*M8Q_TEST_HEADING_GAIN; 

    // Convert heading to degrees 
    heading = (int16_t)(heading_temp*M8Q_TEST_CALC_SCALE/deg_to_rad); 

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

//=======================================================================================
