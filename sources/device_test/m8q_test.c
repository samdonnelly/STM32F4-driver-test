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
// Notes 

// The ACK status function is not explicitly tested here as it's used during init in 
// test 1. 

// The M8Q has no flash to store user settings. Instead they're saved in RAM which can 
// only be powered until the onboard backup battery loses power. For this reason, settings 
// must always be configured in setup. 

//=======================================================================================


//=======================================================================================
// Macros 

// Test 0 
#define M8Q_TEST_0_DATA_BUFF_LIM 300 
#define M8Q_TEST_0_READ_COUNT_LIM 90 
#define M8Q_TEST_0_OVERFLOW_COUNT_LO 40 
#define M8Q_TEST_0_OVERFLOW_COUNT_HI 70 

// Test 1 
#define M8Q_TEST_1_DATA_BUFF_LIM 0 
#define M8Q_TEST_1_COO_STR_LEN 25 
#define M8Q_TEST_1_READ_COUNT_LIM 60 
#define M8Q_TEST_1_LP_COUNT_LIM 90 
#define M8Q_TEST_1_NUM_PARAMS 12 

//=======================================================================================


//=======================================================================================
// Global variables 

// 
typedef struct m8q_test_data_s 
{
    // Test 0 device data 
    uint8_t data_stream[M8Q_TEST_0_DATA_BUFF_LIM]; 

    // Test 1 device data 
    double latitude, longitude; 
    uint8_t lat_str[BYTE_11], lon_str[BYTE_12], utc_time[BYTE_10], utc_date[BYTE_7]; 
    uint8_t NS, EW, navstat_lock; 
    uint16_t navstat; 

    // 
    uint8_t schedule_counter; 
    uint8_t attempt_flag; 
}
m8q_test_data_t; 

static m8q_test_data_t test_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Common/shared setup code 
 */
void m8q_test_general_init(void); 


/**
 * @brief Common/shared test code 
 */
void m8q_test_general(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

// Setup for Test 0 
void m8q_test_0_init(void)
{
    m8q_test_general_init(); 

    // M8Q device setup 
    M8Q_STATUS init_check = m8q_init_dev(
        I2C1, 
        m8q_config_no_pkt, 
        CLEAR, 
        CLEAR, 
        M8Q_TEST_0_DATA_BUFF_LIM); 
    
    // Check if there was a problem during device initialization. If so, output the fault 
    // to the serial terminal and halt to program. 
    if (init_check)
    {
        uart_sendstring(USART2, "\r\nDevice init status: "); 
        uart_send_integer(USART2, (int16_t)init_check); 

        while (TRUE); 
    }
}


// Setup for Test 1 
void m8q_test_1_init(void)
{
    m8q_test_general_init(); 

    // M8Q device setup 
    M8Q_STATUS init_check = m8q_init_dev(
        I2C1, 
        &m8q_config_pkt_0[0][0], 
        M8Q_CONFIG_NUM_MSG_PKT_0, 
        M8Q_CONFIG_MAX_MSG_LEN, 
        M8Q_TEST_1_DATA_BUFF_LIM); 

    // Set up low power and TX ready pins 
    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init_dev(GPIOC, PIN_10); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init_dev(GPIOC, PIN_11); 

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
}


// Common/shared setup code 
void m8q_test_general_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer (for event timing) 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        // 0x0032,  // ARR=50, (50 counts)*(100us/count) = 5ms 
        0x1388,  // ARR=5000, (50 counts)*(100us/count) = 5000ms = 0.5s 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM10); 

    // Initialize UART (serial terminal output) 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Initialize I2C (to communicate with device) 
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

    // Initialize interrupt handler flags and enable the periodic timer interrupt handler 
    int_handler_init(); 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_0); 

    // Screen initialization 
#if M8Q_TEST_SCREEN_ON_BUS 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_display_off(); 
    hd44780u_backlight_off(); 
#endif   // M8Q_TEST_SCREEN_ON_BUS 

    // Initialize variables 
    memset((void *)&test_data, CLEAR, sizeof(test_data)); 

    //==================================================
    // To be assessed 

    // Initialize timers 
    // tim_9_to_11_counter_init(
    //     TIM9, 
    //     TIM_84MHZ_1US_PSC, 
    //     0xFFFF,  // Max ARR value 
    //     TIM_UP_INT_DISABLE); 
    // tim_enable(TIM9); 

    // // User button setup. The user buttons are used to trigger data reads and 
    // // data size checks. 
    // // Initialize the GPIO pins for the buttons and the button debouncer 
    // gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    // gpio_pin_init(GPIOC, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    // debounce_init(GPIOX_PIN_0 | GPIOX_PIN_1 ); 

    // // Delay to let everything finish setup before starting to send and receieve data 
    // tim_delay_ms(TIM9, 500); 
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

// Test 0 code 
void m8q_test_0(void)
{
    // Local variables 
    M8Q_STATUS driver_status; 

    m8q_test_general(); 

    if (test_data.attempt_flag)
    {
        test_data.attempt_flag = CLEAR_BIT; 
        
        if (test_data.schedule_counter < M8Q_TEST_0_READ_COUNT_LIM)
        {
            if ((test_data.schedule_counter < M8Q_TEST_0_OVERFLOW_COUNT_LO) || 
                (test_data.schedule_counter > M8Q_TEST_0_OVERFLOW_COUNT_HI))
            {
                uart_sendstring(USART2, "\nyo"); 

                driver_status = m8q_read_ds_dev(test_data.data_stream, M8Q_TEST_0_DATA_BUFF_LIM); 

                switch (driver_status)
                {
                    case M8Q_OK: 
                        // Output the data stream 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, (char *)test_data.data_stream); 
                        break; 

                    case M8Q_NO_DATA_AVAILABLE: 
                        // Do nothing 
                        break; 

                    case M8Q_DATA_BUFF_OVERFLOW: 
                        // Indicate an overflow (data stream larger than max allowed buffer size) 
                        uart_sendstring(USART2, "\r\nBuffer overflow. Stream cleared.\r\n"); 
                        break; 

                    default:   // Everything else 
                        // Output the fault status 
                        uart_sendstring(USART2, "\r\nDriver fault: "); 
                        uart_send_integer(USART2, (int16_t)driver_status); 
                        uart_send_new_line(USART2); 
                        break; 
                }
            }
            else if (test_data.schedule_counter == M8Q_TEST_0_OVERFLOW_COUNT_LO)
            {
                uart_sendstring(USART2, "\r\nRead pause.\r\n"); 
            }
        }
        // else if (test_data.schedule_counter < 20)
        // {
        //     // Send a message to enter low power mode 
        // }
        else 
        {
            uart_sendstring(USART2, "\nmama"); 
            test_data.schedule_counter = CLEAR; 
        }
    }

#if 0 
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
#endif 
}


// Test 1 code 
void m8q_test_1(void)
{
    // Local variables 
    char latitude_str[M8Q_TEST_1_COO_STR_LEN], longitude_str[M8Q_TEST_1_COO_STR_LEN]; 
    M8Q_STATUS driver_status; 
    GPIO_STATE lp_pin_state = gpio_read(GPIOC, (SET_BIT << PIN_10)); 

    m8q_test_general(); 

    if (test_data.schedule_counter < M8Q_TEST_1_READ_COUNT_LIM)
    {
        m8q_clear_low_pwr_dev(); 

        if (m8q_get_tx_ready_dev())
        {
            driver_status = m8q_read_data_dev(); 

            if (!driver_status)
            {
                test_data.latitude = m8q_get_position_lat_dev(); 
                m8q_get_position_lat_str_dev(test_data.lat_str, BYTE_11); 
                test_data.NS = m8q_get_position_NS_dev(); 
                test_data.longitude = m8q_get_position_lon_dev(); 
                m8q_get_position_lon_str_dev(test_data.lon_str, BYTE_12); 
                test_data.EW = m8q_get_position_EW_dev(); 
                test_data.navstat = m8q_get_position_navstat_dev(); 
                test_data.navstat_lock = m8q_get_position_navstat_lock_dev(); 
                m8q_get_time_utc_time_dev(test_data.utc_time, BYTE_10); 
                m8q_get_time_utc_date_dev(test_data.utc_date, BYTE_7); 

                // Go to the top of the output block in the serial terminal 
                for (uint8_t i = CLEAR; i < M8Q_TEST_1_NUM_PARAMS; i++)
                {
                    uart_sendstring(USART2, "\033[1A"); 
                }

                sprintf(latitude_str, "Latitude: %lf", test_data.latitude); 
                uart_sendstring(USART2, latitude_str); 
                uart_sendstring(USART2, "\r\nLatitude string: "); 
                uart_sendstring(USART2, (char *)test_data.lat_str); 
                uart_sendstring(USART2, "\r\nNS: "); 
                uart_send_integer(USART2, (int16_t)test_data.NS); 
                sprintf(longitude_str, "\r\nLongitude: %lf", test_data.longitude); 
                uart_sendstring(USART2, longitude_str); 
                uart_sendstring(USART2, "\r\nLongitude string: "); 
                uart_sendstring(USART2, (char *)test_data.lon_str); 
                uart_sendstring(USART2, "\r\nEW: "); 
                uart_send_integer(USART2, (int16_t)test_data.EW); 
                uart_sendstring(USART2, "\r\nNAVSTAT: "); 
                uart_send_integer(USART2, (int16_t)test_data.navstat); 
                uart_sendstring(USART2, "\r\nNAVSTAT lock: "); 
                uart_send_integer(USART2, (int16_t)test_data.navstat_lock); 
                uart_sendstring(USART2, "\r\nUTC time: "); 
                uart_sendstring(USART2, (char *)test_data.utc_time); 
                uart_sendstring(USART2, "\r\nUTC date: "); 
                uart_sendstring(USART2, (char *)test_data.utc_date); 
                uart_sendstring(USART2, "\r\nLP state: "); 
                uart_send_integer(USART2, (int16_t)lp_pin_state); 
                uart_sendstring(USART2, "\r\nDriver status: "); 
                uart_send_integer(USART2, (int16_t)driver_status); 
                uart_send_new_line(USART2); 
            }
            else 
            {
                uart_sendstring(USART2, "\033[1A"); 
                uart_sendstring(USART2, "\rDriver status: "); 
                uart_send_integer(USART2, (int16_t)driver_status); 
                uart_send_new_line(USART2); 
            }
        }
    }
    else if (test_data.schedule_counter < M8Q_TEST_1_LP_COUNT_LIM)
    {
        m8q_set_low_pwr_dev(); 

        uart_sendstring(USART2, "\033[1A\033[1A"); 
        uart_sendstring(USART2, "\rLP state: "); 
        uart_send_integer(USART2, (int16_t)lp_pin_state); 
        uart_sendstring(USART2, "\r\n\n"); 
    }
    else 
    {
        test_data.schedule_counter = CLEAR; 
    }
}


// Common/shared test code 
void m8q_test_general(void)
{
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        test_data.schedule_counter++; 
        test_data.attempt_flag = SET_BIT; 
    }
}

//=======================================================================================


//=======================================================================================
// Controller test 

#if M8Q_CONTROLLER_TEST 

//==================================================
// Global variables 

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

//==================================================


//==================================================
// Setup code 

void m8q_test_init()
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

    // Initialize the device controller 
    m8q_controller_init(TIM9); 

    // Initialize the state machine test code 
    state_machine_init(M8Q_NUM_USER_CMDS); 

    // Delay to let everything finish setup before starting to send and receieve data 
    tim_delay_ms(TIM9, 500); 
}

//==================================================


//==================================================
// Test code 

void m8q_test_app()
{
    // Local variables 

    // General purpose arguments array 
    static char user_args[M8Q_MAX_SETTER_ARGS][STATE_USER_TEST_INPUT]; 

    // Control flags 
    uint8_t arg_convert = CLEAR; 
    uint32_t set_get_status = CLEAR; 
    uint8_t cmd_index = CLEAR; 

    // Data buffers 
    uint16_t navstat = CLEAR; 
    uint8_t deg_min[6]; 
    uint8_t min_frac[6]; 
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
}

//==================================================

#endif   // M8Q_CONTROLLER_TEST

//=======================================================================================


//=======================================================================================
// Test functions - replace these with the gps_calc functions 

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
