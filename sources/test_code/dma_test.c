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


//=======================================================================================
// Globals 

uint16_t adc_data[2];  // Location for the DMA to store ADC values 

//=======================================================================================


//==================================================
// Modes & settings 
// 1. ADC non-continuous & scan mode 
//     - CONT disabled 
//     - SCAN enabled 
//     - Memory increment 
//     - Two ADCs initialized 
//     - Sequence defined 
// 
// 2. ADC continuous & scan mode 
//     - CONT enabled 
//     - SCAN enabled 
//     - Memory increment 
//     - Two ADCs initialized 
//     - Sequence defined 
// 
// 3. ADC continuous & single read mode 
//     - CONT enabled 
//     - SCAN disabled 
//     - Memory fixed 
//     - One ADC initialized 
//     - No sequence definition 
//==================================================


// Setup code
void dma_test_init()
{
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    //==================================================
    // ADC Init 

    // Initialize the ADC port (called once) 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_EOC_EACH, 
        // Enable for ADC scan mode 
        ADC_SCAN_DISABLE, 
        // ADC_SCAN_ENABLE, 
        // Enable for ADC continuous mode 
        // ADC_CONT_DISABLE, 
        ADC_CONT_ENABLE, 
        ADC_DMA_ENABLE, 
        ADC_DDS_ENABLE, 
        ADC_EOC_INT_DISABLE, 
        ADC_OVR_INT_DISABLE); 

    // Initialize the first ADC pin and channel (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 

#if ADC_DMA_SECOND_CHANNEL 

    // Initialize the second ADC pin and channel 
    adc_pin_init(ADC1, GPIOC, PIN_1, ADC_CHANNEL_11, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_11, ADC_SEQ_2); 

    // Set the sequence length (called once and only for more than one channel) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

#endif   // ADC_DMA_SECOND_CHANNEL 

    // Turn the ADC on 
    adc_on(ADC1); 

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
        // Memory increment 
        // DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED, 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_data, 
        (uint16_t)SET_2); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 
    
    //==================================================

    //==================================================
    // Start and ADC conversion - ADC continuous modes 

    adc_start(ADC1); 
    
    //==================================================
} 


// Test code 
void dma_test_app()
{
    //==================================================
    // Trigger the ADC conversion - ADC non-continuous & scan mode only 

#if DMA_TEST_MODE_1 

    // Local variables 
    static uint8_t user_button = 0; 
    static uint8_t button_block = 0; 

    // Read the user button state 
    user_button = gpio_read(GPIOC, GPIOX_PIN_13); 

    // Start the ADC scan conversion through DMA if the button is pressed 
    if (!user_button && !button_block)
    {
        adc_start(ADC1); 
        button_block++; 
        tim_delay_ms(TIM9, 10);  // Wait for button bounce to settle 
    }
    else if (user_button && button_block)
        button_block--; 

#endif   // DMA_TEST_MODE_1 

    //==================================================

    // Display the result to the serial terminal 
    uart_sendstring(USART2, "ADC1_10: "); 
    uart_send_integer(USART2, (int16_t)adc_data[0]); 
    uart_send_spaces(USART2, 5); 
    uart_sendstring(USART2, "ADC1_11: "); 
    uart_send_integer(USART2, (int16_t)adc_data[1]); 
    uart_send_new_line(USART2); 

    // Delay 
    tim_delay_ms(TIM9, 1000); 
}
