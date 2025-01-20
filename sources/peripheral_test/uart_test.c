/**
 * @file uart_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief UART test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - UART 
 *            * One UART port must be configured for the serial terminal connection. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * UART drivers are used along with some functions for circular buffer 
 *              handling. 
 *          
 *          Procedure 
 *          - 
 * 
 * @version 0.1
 * @date 2023-09-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "uart_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define UART_TEST_MAX_INPUT 30        // Max user input size (bytes) 

//=======================================================================================


//=======================================================================================
// Global variables 

// UART information 
typedef struct uart_data_s
{
    USART_TypeDef *uart; 
    uint8_t data_buff[UART_TEST_MAX_INPUT];   // Buffer to store serial terminal input 
}
uart_data_t; 

static uart_data_t uart_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// User prompt 
void uart_test_user_prompt(void); 

//=======================================================================================


//=======================================================================================
// Setup code

void uart_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

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

    // Initialize data 
    uart_data.uart = USART2; 
    memset((void *)uart_data.data_buff, CLEAR, sizeof(uart_data.data_buff)); 

    // Provide an initial prompt to the user 
    uart_test_user_prompt(); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void uart_test_app(void)
{
    // Wait until there is data available to be read. The code poles for received data 
    // because it will come at an unknown time and if the data is not read fast enough 
    // then some data will be lost. Once data is available then it gets read. 
    while(!uart_data_ready(uart_data.uart)); 
    uart_get_data(uart_data.uart, uart_data.data_buff); 

    // Echo the received data back to the serial terminal to show the user that the data 
    // was successfully read. Also provide some additional information to demonstrate 
    // other driver functions. 

    uart_test_user_prompt(); 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// User prompt 
void uart_test_user_prompt(void)
{
    uart_send_str(uart_data.uart, "\r\n>>> "); 
}

//=======================================================================================
