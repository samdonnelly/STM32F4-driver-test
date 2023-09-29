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

//=======================================================================================


//=======================================================================================
// Global variables 

// Buffer where DMA UART data gets stored 
static uint8_t uart_dma_buff[30]; 

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
        UART_DMA_TX_DISABLE, 
        UART_DMA_RX_ENABLE); 

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
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)uart_dma_buff, 
        (uint16_t)30); 

    // Enable transfer complete interrupts 
    dma_int_config(
        DMA1_Stream5, 
        DMA_TCIE_ENABLE, 
        DMA_HTIE_DISABLE, 
        DMA_TEIE_DISABLE, 
        DMA_DMEIE_DISABLE); 

    // Enable the DMA stream 
    dma_stream_enable(DMA1_Stream5); 
    
    //===================================================

    //==================================================
    // Initialize interrupts 

    // Enable the interrupt handlers (called for each interrupt) 
    nvic_config(DMA1_Stream5_IRQn, EXTI_PRIORITY_0); 

    //==================================================

    //==================================================
    // Initialize variables 

    memset((void *)uart_dma_buff, CLEAR, sizeof(uart_dma_buff)); 

    //==================================================

    uart_sendstring(USART2, "\r\n>>> "); 

} 

//=======================================================================================


//=======================================================================================
// Test code 

void uart_test_app(void)
{
    // If new UART DMA data is available in the buffer then print it to the terminal 
    if (handler_flags.dma1_5_flag)
    {
        // Reset the DMA1 Stream 5 interrupt flag 
        handler_flags.dma1_5_flag = CLEAR; 

        uart_send_new_line(USART2); 
        uart_sendstring(USART2, (char *)uart_dma_buff); 
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "\r\n>>> "); 
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
