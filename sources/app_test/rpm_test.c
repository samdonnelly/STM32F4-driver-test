/**
 * @file rpm_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief RPM (revolutions per minute) test 
 * 
 * @version 0.1
 * @date 2024-10-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "rpm_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Setup code 

// RPM test setup code 
void rpm_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Initialize UART - used to show the user the calculated RPM 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // External interrupt setup 
}

//=======================================================================================


//=======================================================================================
// Test code 

// RPM test application code 
void rpm_test_app(void)
{
    // 
}

//=======================================================================================
