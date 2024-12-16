/**
 * @file sik_radio_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK radio driver test 
 * 
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "sik_radio_test.h" 
#include "stm32f4xx_it.h" 

extern "C"
{
    // For C headers without C++ guards 
    #include "standard/mavlink.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define SIK_TEST_MSG_BUFF_SIZE 200 

//=======================================================================================


//=======================================================================================
// Global data 

// Mavlink data 
typedef struct mavlink_data_s 
{
    int channel; 
    mavlink_message_t msg; 
    char msg_buff[SIK_TEST_MSG_BUFF_SIZE];   // Should this be a circular buffer 
    uint16_t msg_buff_index; 
    mavlink_status_t status; 
}
mavlink_data_t; 

static mavlink_data_t mavlink_data; 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    // Initialize data 
    mavlink_data.channel = MAVLINK_COMM_0; 
    memset((void *)mavlink_data.msg_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    mavlink_data.msg_buff_index = CLEAR; 

    // Initialize GPIO ports 
    gpio_port_init(); 

    //==================================================
    // UART init 

    // UART1 init - SiK radio module 
    uart_init(
        USART1, 
        GPIOA, 
        PIN_10, 
        PIN_9, 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART1 interrupt init - SiK radio module - IDLE line interrupts 
    uart_interrupt_init(
        USART1, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // UART2 init - Serial terminal 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART2 interrupt init - Serial terminal - IDLE line interrupts 
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

    //==================================================
    // DMA 

    // DMA - UART1 

    // DMA stream init - UART2 
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

    // DMA stream config - UART2 
    // dma_stream_config(
    //     DMA1_Stream5, 
    //     (uint32_t)(&USART2->DR), 
    //     (uint32_t)uart_dma_buff, 
    //     (uint32_t)NULL, 
    //     (uint16_t)UART_TEST_MAX_INPUT); 

    // Enable DMA streams 
    dma_stream_enable(DMA1_Stream5); 

    //==================================================
    
    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void sik_radio_test_app(void)
{
    // Read data somehow. Could be polling, interrupt or DMA. 

    if (1)   // If new data has been read 
    {
        // This does only a single byte at a time. 
        if (mavlink_parse_char(
                mavlink_data.channel, 
                mavlink_data.msg_buff[mavlink_data.msg_buff_index], 
                &mavlink_data.msg, 
                &mavlink_data.status))
        {
            // Message received 
            // Decode the message 
        }

        // When do we increment in the index? 
    }
}

//=======================================================================================
