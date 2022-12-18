/**
 * @file timers_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Timers test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "timers_test.h"

//=======================================================================================


//=======================================================================================
// Global variables 

static uint32_t no_block_delay_count_total = CLEAR; 
static uint32_t no_block_delay_count_compare = CLEAR; 
static uint8_t  no_block_delay_start_flag = SET_BIT; 

//=======================================================================================


// Setup code
void timers_test_init()
{
    // Setup code for the timers_test here 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_2_to_5_output_init(
        TIM3, 
        TIM_CHANNEL_1, 
        GPIOC, 
        PIN_6, 
        TIM_DIR_UP, 
        PWM_ARR, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH); 
    
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x2710,  // ARR=10000, (10000 counts)*(100us/count) = 1s 
        TIM_UP_INT_ENABLE); 

    // tim_9_to_11_counter_init(
    //     TIM9, 
    //     TIM_84MHZ_50US_PSC, 
    //     0x4E20,  // ARR=20000, (20000 counts)*(50us/count) = 1s 
    //     TIM_UP_INT_ENABLE); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_0);  // TIM9 update 

    // Enable timers 
    tim_enable(TIM3); 
    tim_enable(TIM9); 

    //==================================================
    // Check system clock frequencies 

    uint32_t pclk1_frq = (rcc_get_pclk1_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t pclk2_frq = (rcc_get_pclk2_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t hclk_frq  = (rcc_get_hclk_frq()  / DIVIDE_1000) / DIVIDE_1000; 

    uart_sendstring(USART2, "PCLK1 Freq: "); 
    uart_send_integer(USART2, (int16_t)pclk1_frq); 
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "PCLK2 Freq: "); 
    uart_send_integer(USART2, (int16_t)pclk2_frq); 
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "HCLK Freq:  "); 
    uart_send_integer(USART2, (int16_t)hclk_frq); 
    uart_send_new_line(USART2); 

    //==================================================
} 


// Test code 
void timers_test_app()
{
    // Test code for the timers_test here 

    // Local variables 
    static uint16_t counter = 0; 
    static uint16_t pwm = 0; 
    static uint8_t pwm_dir = 1; 

    //==================================================
    // Periodic interrupt test 

    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        // Clear interrupt handler 
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        //==================================================
        // PWM output test 

        // Set PWM output and calculate a new value 
        tim_ccr(TIM3, pwm, TIM_CHANNEL_1); 
        if (pwm_dir)  // Counting up 
        {
            if (pwm < PWM_ARR) 
                pwm += PWM_INCREMENT; 
            else 
            {
                pwm_dir = 1 - pwm_dir; 
                pwm -= PWM_INCREMENT; 
            }
        }

        else  // Counting down 
        {
            if (pwm > NONE) 
                pwm -= PWM_INCREMENT; 
            else 
            {
                pwm_dir = 1 - pwm_dir; 
                pwm += PWM_INCREMENT; 
            }
        }

        //==================================================

        // Print the counter to the terminal 
        uart_sendstring(USART2, "Counter: "); 
        uart_send_integer(USART2, (int16_t)counter); 
        uart_send_new_line(USART2); 
        counter++; 
    }

    //==================================================
    
    //==================================================
    // Non-blocking delay test 

    if (tim_time_compare(TIM9, 
                         TIM_NO_BLOCK_DELAY, 
                         &no_block_delay_count_total, 
                         &no_block_delay_count_compare, 
                         &no_block_delay_start_flag))
    {
        // Print to the terminal to verify that the delay works 
        uart_sendstring(USART2, "Delay!"); 
        uart_send_new_line(USART2);

        // Reset the start bit 
        no_block_delay_start_flag = SET_BIT; 
    }

    //==================================================
}
