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
// Globals 

#if M8Q_CONTROLLER_TEST

// User command table 
static state_request_t m8q_state_cmds[M8Q_NUM_USER_CMDS] = 
{
    {"lp_set",   0, M8Q_SETTER_PTR_1, 0}, 
    {"lp_clear", 0, M8Q_SETTER_PTR_1, 0}, 
    {"reset",    0, M8Q_SETTER_PTR_1, 0}, 
    {"state",    0, M8Q_GETTER_PTR_1, 0}, 
    {"navstat",  0, M8Q_GETTER_PTR_1, 0}, 
    {"execute", 0, 0, 0} 
}; 


// Function pointer table 
static m8q_func_ptrs_t m8q_state_func[M8Q_NUM_USER_CMDS] = 
{
    {&m8q_set_low_pwr_flag, NULL}, 
    {&m8q_clear_low_pwr_flag, NULL}, 
    {&m8q_set_reset_flag, NULL}, 
    {NULL, &m8q_get_state}, 
    {NULL, &m8q_get_nav_state}, 
    {NULL, NULL} 
}; 

#endif 

//=======================================================================================


// Setup code
void m8q_test_init()
{
    //===================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    //===================================================

    //===================================================
    // Timers 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

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

#endif   // M8Q_DATA_CHECK 

    //===================================================

    //===================================================
    // Peripherals 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 

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

    //===================================================

#if M8Q_DATA_CHECK 

    //===================================================
    // User button setup 

    // The user buttons are used to trigger data reads and data size checks 

    // Initialize the GPIO pins for the buttons 
    gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    gpio_pin_init(GPIOC, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

    // Initialize the button debouncer 
    debounce_init(GPIOX_PIN_0 |GPIOX_PIN_1 ); 

    //===================================================

#endif   // M8Q_DATA_CHECK 

    //==================================================+
    // M8Q test mode selection 

#if M8Q_USER_CONFIG 

    m8q_user_config_init(I2C1); 

#else

    // M8Q device setup 
    char m8q_config_messages[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN]; 
    m8q_config_copy(m8q_config_messages); 

    m8q_init(
        I2C1, 
        GPIOC, 
        PIN_10, 
        PIN_11, 
        M8Q_CONFIG_MSG_NUM, 
        M8Q_CONFIG_MSG_MAX_LEN, 
        (uint8_t *)m8q_config_messages[0]); 
    
    if (m8q_get_status()) 
    {
        uart_sendstring(USART2, "M8Q init fault.\r\n"); 
    }

#if M8Q_CONTROLLER_TEST

    // Initialize the device controller 
    m8q_controller_init(TIM9); 

    // Initialize the state machine test code 
    state_machine_init(M8Q_NUM_USER_CMDS); 

#endif   // M8Q_CONTROLLER_TEST 

#endif   // M8Q_USER_CONFIG 

    //==================================================+

    // Delay to let everything finish setup before starting to send and receieve data 
    tim_delay_ms(TIM9, 500); 
} 


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
    uint8_t arg_convert = 0; 
    uint32_t set_get_status = 0; 
    uint8_t cmd_index = 0; 
    uint8_t state = 0; 

    // Determine what to do from user input 
    state_machine_test(m8q_state_cmds, user_args[0], &cmd_index, &arg_convert, &set_get_status); 

    // Check if there are any setters or getters requested 
    if (set_get_status)
    {
        for (uint8_t i = 0; i < (M8Q_NUM_USER_CMDS-1); i++)
        {
            if ((set_get_status >> i) & SET_BIT)
            {
                switch (m8q_state_cmds[i].func_ptr_index)
                {
                    case M8Q_SETTER_PTR_1: 
                        (m8q_state_func[i].setter)(); 
                        break; 

                    case M8Q_GETTER_PTR_1: 
                        state = (m8q_state_func[i].getter)(); 
                        uart_sendstring(USART2, "\n"); 
                        uart_send_integer(USART2, (int16_t)state); 
                        uart_send_new_line(USART2); 

                    default: 
                        break; 
                }
            }
        }
    }

    // Call the device controller 
    m8q_controller(); 

    //==================================================

#else 

    //==================================================
    // Local variables 

#if M8Q_MSG_COUNT 

    uint8_t count = CLEAR; 

#endif   // M8Q_MSG_COUNT 

#if M8Q_DATA_CHECK 

    static uint8_t button_block_1 = CLEAR; 
    static uint8_t button_block_2 = CLEAR; 
    uint16_t data_size; 
    uint8_t current_time[10]; 

#endif   // M8Q_DATA_CHECK 

#if M8Q_TEST_OTHER 

    uint8_t counter = CLEAR; 
    static uint8_t flipper = CLEAR; 
    static uint16_t timer = 0x48B8; 
    static uint8_t blink = 0; 

    // M8Q data 
    uint16_t lat_deg_min = CLEAR; 
    uint32_t lat_min_frac = CLEAR; 
    volatile uint8_t NS = CLEAR; 
    uint16_t lon_deg_min = CLEAR; 
    uint32_t lon_min_frac = CLEAR; 
    volatile uint8_t EW = CLEAR; 
    volatile uint16_t navstat = CLEAR; 
    uint8_t utc_time[BYTE_9]; 
    uint8_t utc_date[BYTE_6]; 

#endif   // M8Q_TEST_OTHER 

    //==================================================

    //===================================================
    // Data record, power save mode and TX-Ready testing 
    
#if M8Q_MSG_COUNT 

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

#if M8Q_TEST_OTHER 

    while (TRUE)
    {
        if (m8q_get_tx_ready())
        {
            // Read the data 
            m8q_read(); 
            counter++; 

            // Blink the board LED for visual feedback 
            blink = GPIO_HIGH - blink; 
            gpio_write(GPIOA, GPIOX_PIN_5, blink); 
        }
        else
        {
            if (counter == 2)
            {
                m8q_get_lat(&lat_deg_min, &lat_min_frac); 
                NS = m8q_get_NS(); 
                m8q_get_long(&lon_deg_min, &lon_min_frac); 
                EW = m8q_get_EW(); 
                navstat = m8q_get_navstat(); 
                m8q_get_time(utc_time); 
                m8q_get_date(utc_date); 
                counter = 0; 
            }
            break; 
        }
    }

    // Toggle the EXTINT pin to set low power mode 
    if (!(--timer))
    {
        m8q_set_low_power(flipper); 
        flipper = GPIO_HIGH - flipper; 
        timer = 0x68B8; 
        tim_delay_ms(TIM9, 150);  // Give time for the receiver to startup from sleep mode 

        // The following line of code was needed in order for the TX_READY input pin to start 
        // functioning normally again after low power mode. Make this part of a state when 
        // returning from low power mode.  
        if (!flipper) while (!(m8q_read())); 
    }

    tim_delay_ms(TIM9, 1); 

#endif   // M8Q_TEST_OTHER 

    //===================================================

#endif   // M8Q_CONTROLLER_TEST

#endif   // M8Q_USER_CONFIG
}
