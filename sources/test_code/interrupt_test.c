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

    // Enable external interrupts (called once) 
    exti_init(); 

    // Configure the external interrupt (called for each EXTI) 
    exti_config(GPIOD, 
                EXTI_PD, 
                PIN_2, 
                PUPDR_PU, 
                EXTI_L2, 
                EXTI_INT_NOT_MASKED, 
                EXTI_EVENT_MASKED, 
                EXTI_RISE_TRIG_ENABLE, 
                EXTI_FALL_TRIG_DISABLE); 

    //==================================================

    //==================================================
    // ADC init --> Include ADC interrupts 
    //================================================== 

    //==================================================
    // DMA init --> Include DMA interrupts 
    //==================================================

    //==================================================
    // Interrupts init 

    // Initialize handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) 
    nvic_config(EXTI2_IRQn, EXTI_PRIORITY_0);  // EXTI2 

    //==================================================
}


// Test code 
void int_test_app()
{
    // Interrupt application code goes here 

    // Local variables 
    static uint8_t exti_counter = 0; 

    // Timer interrupt 

    // External interrupt 
    if (handler_flags.exti2_flag)
    {
        handler_flags.exti2_flag = CLEAR; 
        exti_counter++; 
        uart_sendstring(USART2, "EXTI2: "); 
        uart_send_integer(USART2, (int16_t)exti_counter); 
        uart_send_new_line(USART2); 
    }

    // ADC interrupt 

    // DMA interrupt 
}
