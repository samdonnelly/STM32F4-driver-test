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
// Setup code

void dma_test_init(void)
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
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // // Configure the DMA stream 
    // dma_stream_config(
    //     DMA2_Stream0, 
    //     (uint32_t)(&ADC1->DR), 
    //     (uint32_t)adc_data, 
    //     (uint16_t)ADC_BUFF_SIZE); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 
    
    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void dma_test_app(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
