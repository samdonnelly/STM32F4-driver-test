/**
 * @file interrupt_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt test code 
 * 
 * @version 0.1
 * @date 2022-11-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//================================================================================
// Includes 

#include "interrupt_test.h" 

//================================================================================


// Setup code 
void int_test_init()
{
    // Interrupt initialization code goes here 

    //==================================================
    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Timer interrupt enable 
    
    //==================================================

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    //==================================================
    // EXTI init 
    //==================================================

    //==================================================
    // ADC Init --> Include ADC interrupts 
    //================================================== 

    //==================================================
    // DMA init --> Include DMA interrupts 
    //==================================================
}


// Test code 
void int_test_app()
{
    // Interrupt application code goes here 

    // Timer interrupt 

    // External interrupt 

    // ADC interrupt 

    // DMA interrupt 
}
