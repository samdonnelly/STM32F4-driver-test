/**
 * @file uart_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief UART test code 
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

// A circular buffer is used to store a continuous stream of UART (serial terminal) inputs 
// and is also used to parse the most recent user input which gets stored in a user input 
// buffer. 
static uint8_t uart_dma_buff[UART_TEST_MAX_INPUT];     // Circular buffer for uart inputs 
static uint8_t user_input_buff[UART_TEST_MAX_INPUT];   // Stores latest user input 
static uint8_t buff_index;                             // Circular buffer index 

//=======================================================================================


//=======================================================================================
// Setup code

void uart_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    //==================================================
    // UART initialization 

    // Initialize UART 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 
    
    // Enable IDLE line interrupts - this interrupt is triggered when the RX line 
    // goes idle (no more data coming in). This is used to read variable length 
    // UART inputs via DMA. If DMA transfer interrupts are used then the interrupt 
    // will only trigger for fixed lengths. 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    //==================================================

    //===================================================
    // Initialize DMA 

    // Initialize the DMA stream 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)uart_dma_buff, 
        (uint32_t)NULL, 
        (uint16_t)UART_TEST_MAX_INPUT); 

    // Enable the DMA stream 
    dma_stream_enable(DMA1_Stream5); 
    
    //===================================================

    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    //==================================================

    //==================================================
    // Initialize variables 

    memset((void *)uart_dma_buff, CLEAR, sizeof(uart_dma_buff)); 
    memset((void *)user_input_buff, CLEAR, sizeof(user_input_buff)); 
    buff_index = CLEAR; 

    //==================================================

    uart_sendstring(USART2, "\r\n>>> "); 

} 

//=======================================================================================


//=======================================================================================
// Test code 

void uart_test_app(void)
{
    // This interrupt flag will be set when an idle line is detected on UART RX after 
    // receiving new data. This new data gets echoed back over the UART. 
    if (handler_flags.usart2_flag)
    {
        // Reset the USART2 interrupt flag 
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(uart_dma_buff, user_input_buff, &buff_index, UART_TEST_MAX_INPUT); 

        // Echo the user input back to the terminal 
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, (char *)user_input_buff); 
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "\r\n>>> "); 
    }
}

//=======================================================================================
