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
//=======================================================================================


//=======================================================================================
// Global variables 
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
}

//=======================================================================================


//=======================================================================================
// Test code 

void esc_readytosky_test_app(void)
{
    // Local variables 

    // Ready UART user input 
    // If new input date then update output 

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
//=======================================================================================

