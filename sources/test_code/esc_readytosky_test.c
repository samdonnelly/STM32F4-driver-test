/**
 * @file esc_readytosky_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky ESC test code 
 * 
 * @version 0.1
 * @date 2023-09-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "esc_readytosky_test.h" 

//=======================================================================================


//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief User input check and conversion 
 * 
 * @param input_buff : stores to raw user input 
 * @param input_num : stores the converted number input 
 * @return uint8_t : status of the input check --> TRUE : valid, FALSE : invalid 
 */
uint8_t esc_test_input_check(
    char *input_buff, 
    uint32_t *input_num); 

//=======================================================================================


//=======================================================================================
// Global variables 

// User data 
static char cmd_buff[ESC_INPUT_BUF_LEN]; 
static uint32_t pwm_input; 

//=======================================================================================


//=======================================================================================
// Setup code 

void esc_readytosky_test_init(void)
{
    //===================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 
    
    //===================================================

#if ESC_PARAM_ID 

    // PWM output timer 
    tim_2_to_5_output_init(
        TIM3, 
        TIM_CHANNEL_4, 
        GPIOB, 
        PIN_1, 
        TIM_DIR_UP, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 

    tim_enable(TIM3); 

    // Set the initial PWM value 
    tim_ccr(TIM3, ESC_NEUTRAL_TIME, TIM_CHANNEL_4); 

#else   // ESC_PARAM_ID 

    // ESC driver setup 
    esc_readytosky_init(
        DEVICE_ONE, 
        TIM3, 
        TIM_CHANNEL_4, 
        GPIOB, 
        PIN_1, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#if ESC_SECOND_DEVICE 

    esc_readytosky_init(
        DEVICE_TWO, 
        TIM3, 
        TIM_CHANNEL_3, 
        GPIOB, 
        PIN_0, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#endif   // ESC_SECOND_DEVICE 

    // Enable the PWM timer(s) 
    tim_enable(TIM3); 

#endif   // ESC_PARAM_ID 

    //===================================================
    // Variable init 

    memset((void *)cmd_buff, CLEAR, sizeof(cmd_buff)); 
    pwm_input = CLEAR; 

    //===================================================

    //===================================================
    // User init 

    // Send a user prompt to the terminal 
    uart_sendstring(USART2, ">>> "); 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void esc_readytosky_test_app(void)
{
    // Local variables 

    // Check if data is available to be read 
    if (uart_data_ready(USART2))
    {
        // Read the available data 
        uart_getstr(USART2, cmd_buff, ESC_INPUT_BUF_LEN, UART_STR_TERM_CARRIAGE); 

        // Clear the data register of any additional data 
        while (uart_data_ready(USART2))
        {
            uart_clear_dr(USART2); 
        }

        // Check that the input is a valid number 
        if (esc_test_input_check(cmd_buff, &pwm_input))
        {
            // Compare the input to the allowable input range 
            if (pwm_input > ESC_FWD_SPEED_LIM)
            {
                pwm_input = ESC_FWD_SPEED_LIM; 
            }
            else if (pwm_input < ESC_REV_SPEED_LIM)
            {
                pwm_input = ESC_REV_SPEED_LIM; 
            }

            // Update the PWM value 
            tim_ccr(TIM3, pwm_input, TIM_CHANNEL_4); 
        }
        else 
        {
            uart_sendstring(USART2, "\r\nInvalid input\r\n"); 
        }

        // Send a user prompt to the terminal 
        uart_sendstring(USART2, "\r\n>>> "); 
    }

#if ESC_PARAM_ID 

#else   // ESC_PARAM_ID 

    // Write PWM command to ESC/motor 
    esc_readytosky_send(); 

#if ESC_SECOND_DEVICE 

#endif   // ESC_SECOND_DEVICE 

#endif   // ESC_PARAM_ID 
}

//=======================================================================================


//=======================================================================================
// Test functions 

// User input check and conversion 
uint8_t esc_test_input_check(
    char *input_buff, 
    uint32_t *input_num)
{
    // Local varaibles 
    uint8_t input_len = CLEAR; 
    uint32_t digit = CLEAR; 

    // Clear the previous input number conversion 
    *input_num = CLEAR; 

    // Check that all the characters are digits and get the input length 
    for (uint8_t i = 0; i < ESC_INPUT_BUF_LEN; i++)
    {
        if (input_buff[i] == NULL_CHAR)
        {
            break; 
        }

        input_len++; 

        // If the character is not a digit then it's an invalid input 
        if ((input_buff[i] < ZERO_CHAR) && (input_buff[i] > NINE_CHAR))
        {
            return FALSE; 
        }
    }

    // Convert the input to a number 
    for (uint8_t i = 0; i < input_len; i++)
    {
        digit = (uint32_t)(input_buff[i] - NUM_TO_CHAR_OFFSET); 
        *input_num += digit*(uint32_t)pow((double)10, (double)(input_len-i-1)); 
    }

    return TRUE; 
}

//=======================================================================================
