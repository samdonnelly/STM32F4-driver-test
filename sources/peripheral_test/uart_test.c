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

// Data structure to hold UART circular buffer data. The circular buffer gets populated 
// by DMA when UART data is received and that UART data is then parsed into a separate 
// buffer to make it available for the application. 
typedef struct uart_dma_cb_s
{
    USART_TypeDef *uart; 
    DMA_TypeDef *dma_stream; 
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
        (uint32_t)cb.cb, 
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

    //==================================================

    uart_test_user_prompt(); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void uart_test_app(void)
{
    // When an idle line interrupt occurs (i.e. UART RX line goes idle for too long) 
    // indicating the end of the received serial terminal data, the data will be parsed 
    // and echoed back to the serial terminal to verify that it has been parsed 
    // correctly. The number of data items transferred by DMA is first found before 
    // the circular buffer can be parsed. 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 
        
        dma_cb_index(cb.dma_stream, &cb.dma_index, &cb.cb_index); 
        cb_parse(cb.cb, &cb.cb_index, cb.data_buff); 

        uart_send_new_line(cb.uart); 
        uart_sendstring(cb.uart, (char *)cb.data_buff); 
        uart_send_new_line(cb.uart); 
        uart_test_user_prompt(); 
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// User prompt 
void uart_test_user_prompt(void)
{
    uart_sendstring(cb.uart, "\r\n>>> "); 
}

//=======================================================================================
