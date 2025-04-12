/**
 * @file circular_buffer_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Circular buffer test 
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
 *          - DMA 
 *            * DMA is configured for the RX line so data from the serial terminal 
 *              will automatically be stored to prevent any loss of data. 
 *          - Interrupts 
 *            * An idle line interrupt is configured for the RX line meaning an interrupt 
 *              will be triggered when the RX line goes from receiving data to no longer 
 *              receiving data. This interrupt triggers the start of input processing. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * UART, DMA and interrupt drivers are used along with some functions for 
 *              circular buffer handling. 
 *          
 *          Procedure 
 *          - A prompt is initially provided to the serial terminal where the user can 
 *            input data. After this the code won't do anything until the UART RX idle 
 *            line interrupt is triggered. Once triggered, serial terminal data that 
 *            was placed into a circular buffer by the DMA will be parsed to retreive 
 *            the provided input. This input is then echoed back to the serial terminal 
 *            so the user can see that the circular buffer system works. 
 *          - Two functions are called after data is received. One is a DMA indexing 
 *            function that identifies the numbers of data items that were transferred 
 *            and updated the circular buffer head index. The second is a circular 
 *            buffer parsing function to extract the input into another buffer. These 
 *            are the main pieces of code being tested here aside from the functionality 
 *            of UART + DMA + interrupts. 
 *          - Note that serial terminal inputs should be less than UART_TEST_MAX_INPUT. 
 * 
 * @version 0.1
 * @date 2025-01-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "circular_buffer_test.h" 
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
struct UART_PARAM_cb 
{
    USART_TypeDef *uart; 
    DMA_Stream_TypeDef *dma_stream; 
    uint8_t cb[UART_TEST_MAX_INPUT];          // Circular buffer populated by DMA 
    cb_index_t cb_index;                      // Circular buffer indexing info 
    dma_index_t dma_index;                    // DMA transfer indexing info 
    uint8_t data_buff[UART_TEST_MAX_INPUT];   // Buffer that stores latest UART input 
};

static UART_PARAM_cb cb; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// User prompt 
void uart_test_user_prompt(void); 

//=======================================================================================


//=======================================================================================
// Setup code

void cb_test_init(void)
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
        UART_PARAM_DISABLE, 
        CLEAR, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE); 
    
    // Enable IDLE line interrupts - this interrupt is triggered when the RX line 
    // goes idle (no more data coming in). This is used to read variable length 
    // UART inputs via DMA. If DMA transfer interrupts are used then the interrupt 
    // will only trigger for fixed lengths. 
    uart_interrupt_init(
        USART2, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

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

void cb_test_app(void)
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
        uart_send_str(cb.uart, (char *)cb.data_buff); 
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
    uart_send_str(cb.uart, "\r\n>>> "); 
}

//=======================================================================================
