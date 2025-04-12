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
 *          - Wait for data to be received from the serial terminal via UART RX line. 
 *            Once data is available, read the data, echo the input back to the serial 
 *            terminal and output some random strings to demonstrate UART driver 
 *            functions. 
 *          
 *          Other tests demonstrating the UART driver: 
 *          - circular_buffer_test.cpp - interrupt setup 
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
#define UART_TEST_CHAR_MIN 97         // 97 == "a" 
#define UART_TEST_CHAR_MAX 122        // 122 == "z" 
#define UART_TEST_DIGIT_MIN 0 
#define UART_TEST_DIGIT_MAX 9 

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

// Random number 
uint8_t uart_test_rand(
    uint8_t min, 
    uint8_t max); 

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
        UART_PARAM_DISABLE, 
        CLEAR, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

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
    // volatile uint8_t rand_return = CLEAR; 

    // Wait until there is data available to be read. The code poles for received data 
    // because it will come at an unknown time and if the data is not read fast enough 
    // then some data will be lost. Once data is available then it gets read. Note that 
    // this method is not generally recommended but used to show the driver functions. 
    while(!uart_data_ready(uart_data.uart)); 
    uart_get_data(uart_data.uart, uart_data.data_buff); 

    // Echo the received data back to the serial terminal to show the user that the data 
    // was successfully read. 
    uart_send_str(uart_data.uart, "\r\nEcho: "); 
    uart_send_str(uart_data.uart, uart_data.data_buff); 

    // Provide some additional information to demonstrate other driver functions. The 
    // output should look like the following: 
    // "Characters: <#1> <#2> <#3>" 
    // "Digits: <#1>  <#2>  <#3>" 
    // "Integer: <#1>" 
    // "Formatted str: <#1>" 

    // Separate random output from the echo output 
    uart_send_new_line(uart_data.uart); 
    uart_send_new_line(uart_data.uart); 

    // Generate and send three random characters 
    uart_send_str(uart_data.uart, "Characters: "); 
    uart_send_char(uart_data.uart, uart_test_rand(UART_TEST_CHAR_MIN, UART_TEST_CHAR_MAX)); 
    uart_send_spaces(uart_data.uart, 1); 
    uart_send_char(uart_data.uart, uart_test_rand(UART_TEST_CHAR_MIN, UART_TEST_CHAR_MAX)); 
    uart_send_spaces(uart_data.uart, 1); 
    uart_send_char(uart_data.uart, uart_test_rand(UART_TEST_CHAR_MIN, UART_TEST_CHAR_MAX)); 
    uart_send_new_line(uart_data.uart); 

    // Generate and send three random digits 
    uart_send_str(uart_data.uart, "Digits: "); 
    uart_send_digit(uart_data.uart, uart_test_rand(UART_TEST_DIGIT_MIN, UART_TEST_DIGIT_MAX)); 
    uart_send_spaces(uart_data.uart, 2); 
    uart_send_digit(uart_data.uart, uart_test_rand(UART_TEST_DIGIT_MIN, UART_TEST_DIGIT_MAX)); 
    uart_send_spaces(uart_data.uart, 2); 
    uart_send_digit(uart_data.uart, uart_test_rand(UART_TEST_DIGIT_MIN, UART_TEST_DIGIT_MAX)); 
    uart_send_new_line(uart_data.uart); 

    // Generate and send a random integer 
    uart_send_str(uart_data.uart, "Integer: "); 
    uart_send_integer(
        uart_data.uart, 
        (int16_t)uart_test_rand(ZERO, HIGH_8BIT) - (int16_t)uart_test_rand(ZERO, HIGH_8BIT)); 
    uart_send_new_line(uart_data.uart); 

    // Generate and send a formatted string 
    uart_send_new_line(uart_data.uart);                    // Go down a line 
    uart_cursor_move(uart_data.uart, UART_CURSOR_UP, 1);   // Go back up a line 
    snprintf(
        uart_data.data_buff, 
        UART_TEST_MAX_INPUT, 
        "Formatted str: %d\r\n", 
        (int16_t)uart_test_rand(ZERO, HIGH_8BIT) - (int16_t)uart_test_rand(ZERO, HIGH_8BIT)); 
    uart_send_str(uart_data.uart, (char *)uart_data.data_buff); 

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


// Random number 
uint8_t uart_test_rand(
    uint8_t min, 
    uint8_t max)
{
    return (uint8_t)(rand() % (max - min + 1) + min); 
}

//=======================================================================================
