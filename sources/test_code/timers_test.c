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


// Setup code
void timers_test_init()
{
    // Setup code for the timers_test here 
    
    // Initialize timers 
    // tim_9_to_11_init(
    //     TIM9, 
    //     TIM_CHANNEL_1, 
    //     TIMERS_APB2_84MHZ_100US_PRESCALAR, 
    //     0x2710,  // ARR=10000, 10000*100us*84MHz = ~1s 
    //     TIM_OCM_FROZEN, 
    //     TIM_OCPE_DISABLE, 
    //     TIM_ARPE_DISABLE, 
    //     TIM_CCP_AH, 
    //     TIM_CCE_OFF, 
    //     TIM_UP_INT_ENABLE); 
    
    tim_9_to_11_counter_init(
        TIM9, 
        TIMERS_APB2_84MHZ_100US_PRESCALAR, 
        0x2710,  // ARR=10000, 10000*100us*84MHz = ~1s 
        TIM_UP_INT_ENABLE); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_0);  // TIM9 update 

    // Enable timers 
    tim_enable(TIM9); 

} 


// Test code 
void timers_test_app()
{
    // Test code for the timers_test here 

    // Local variables 
    static uint16_t counter = 0; 

    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 
        uart_sendstring(USART2, "Counter: "); 
        uart_send_integer(USART2, (int16_t)counter); 
        uart_send_new_line(USART2); 
        counter++; 
    }
}
