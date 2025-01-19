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
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define UART_TEST_MAX_INPUT 30        // Max user input size (bytes) 

//=======================================================================================


//=======================================================================================
// Global variables 

// Data structure to hold UART circular buffer data. The circular buffer gets populated 
// by DMA when UART data is received and that UART data is then parsed into a separate 
// buffer to make it available for the application. 
typedef struct uart_dma_cb_s
{
    USART_TypeDef *uart; 
    DMA_Stream_TypeDef *dma_stream; 
    uint8_t cb[UART_TEST_MAX_INPUT];          // Circular buffer populated by DMA 
    cb_index_t cb_index;                      // Circular buffer indexing info 
    dma_index_t dma_index;                    // DMA transfer indexing info 
    uint8_t data_buff[UART_TEST_MAX_INPUT];   // Buffer that stores latest UART input 
}
uart_dma_cb_t; 

static uart_dma_cb_t cb; 

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
    cb.uart = USART2; 
    cb.dma_stream = DMA1_Stream5; 
    memset((void *)cb.cb, CLEAR, sizeof(cb.cb)); 
    cb.cb_index.cb_size = UART_TEST_MAX_INPUT; 
    cb.cb_index.head = CLEAR; 
    cb.cb_index.tail = CLEAR; 
    cb.dma_index.data_size = CLEAR; 
    cb.dma_index.ndt_old = dma_ndt_read(cb.dma_stream); 
    cb.dma_index.ndt_new = CLEAR; 
    memset((void *)cb.data_buff, CLEAR, sizeof(cb.data_buff)); 

    // Provide an initial prompt to the user 
    uart_test_user_prompt(); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void uart_test_app(void)
{
    // Look for data 
    uart_get_data(cb.uart, cb.data_buff); 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// User prompt 
void uart_test_user_prompt(void)
{
    uart_send_str(cb.uart, "\r\n>>> "); 
}

//=======================================================================================
