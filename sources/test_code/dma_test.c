/**
 * @file dma_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief DMA test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "dma_test.h"

//=======================================================================================


//================================================================================
// Globals 

uint16_t adc_data[2];  // Location for the DMA to store ADC values 

//================================================================================


// Setup code
void dma_test_init()
{
    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    //==================================================
    // ADC Init 

    // Initialize the ADC port (called once) 
    adc_port_init(ADC1, 
                  ADC1_COMMON, 
                  ADC_PCLK2_4, 
                  ADC_RES_8, 
                  ADC_EOC_EACH, 
                  ADC_SCAN_ENABLE, 
                  ADC_CONT_ENABLE, 
                  ADC_DMA_ENABLE, 
                  ADC_DDS_ENABLE); 

    // Initialize ADC pins and channels (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 
    adc_pin_init(ADC1, GPIOC, PIN_1, ADC_CHANNEL_11, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 
    adc_seq(ADC1, ADC_CHANNEL_11, ADC_SEQ_2); 

    // Set the sequence length (called once) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

    //================================================== 

    //==================================================
    // DMA init 

    // Initialize the DMA stream 
    dma_stream_init(
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE, 
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT,   // Memory increment - new data saved in new location 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF,
        DMA_FTH_1QTR, 
        DMA_DIRECT_MODE); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_data, 
        (uint16_t)SET_2); 
    
    //==================================================

    //==================================================
    // Start the ADC after the DMA is set up 

    // Turn the ADC on 
    adc_on(ADC1); 

    // Start and ADC conversion 
    adc_start(ADC1); 

    //==================================================
} 


// Test code 
void dma_test_app()
{
    // Local variables 

    // Read ADC 

    // Display the result to the serial terminal 
    uart_sendstring(USART2, "ADC1_10: "); 
    uart_send_integer(USART2, (int16_t)adc_data[0]); 
    uart_send_spaces(USART2, 5); 
    uart_sendstring(USART2, "ADC1_11: "); 
    uart_send_integer(USART2, (int16_t)adc_data[1]); 
    uart_send_new_line(USART2); 

    // Delay 
    tim9_delay_ms(1000); 
}
