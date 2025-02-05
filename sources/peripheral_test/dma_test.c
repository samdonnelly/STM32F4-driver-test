/**
 * @file dma_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief DMA test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *          
 *          Configuration 
 *          - 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - 
 *          
 *          Procedure 
 *          - 
 *          
 *          Other tests demonstrating the analog driver: 
 *          - 
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
// Macros 

#define ADC_TEST_BUFF_SIZE 3        // Size according to the number of ADCs used 
#define ADC_TEST_PRINT_SPACES 5     // Spaces between values displayed in the terminal 
#define ADC_TEST_LOOP_DELAY 1000    // Blocking delay time for test loop 
#define ADC_TEST_CONVERT_DELAY 10   // Blocking delay time for ADC conversion 

//=======================================================================================


//=======================================================================================
// Enums 

// ADC index 
typedef enum {
    FIRST_ADC, 
    SECOND_ADC, 
    THIRD_ADC
} adc_index_t; 

//=======================================================================================


//=======================================================================================
// Globals 

typedef struct dma_test_data_s
{
    TIM_TypeDef *timer; 
    USART_TypeDef *uart; 
    uint16_t adc_data[ADC_TEST_BUFF_SIZE]; 
}
dma_test_data_t; 

static dma_test_data_t dma_data; 

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


//=======================================================================================
// Setup code

void dma_test_init()
{
    // Initialize data 
    dma_data.timer = TIM9; 
    dma_data.uart = USART2; 
    memset((void *)dma_data.adc_data, CLEAR, sizeof(dma_data.adc_data)); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        dma_data.timer, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(dma_data.timer); 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        dma_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    //==================================================
    // ADC Init 

    // Initialize the ADC port (called once) 
    adc1_clock_enable(RCC); 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
#if DMA_TEST_MODE_3   // ADC scan mode 
        ADC_PARAM_DISABLE, 
#else 
        ADC_PARAM_ENABLE, 
#endif 
#if DMA_TEST_MODE_1  // ADC continuous mode 
        ADC_PARAM_DISABLE, 
#else 
        ADC_PARAM_ENABLE, 
#endif 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize the first ADC pin and channel (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOA, PIN_6, ADC_CHANNEL_6, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_6, ADC_SEQ_1); 

#if ADC_DMA_SECOND_CHANNEL 

    // Initialize the second ADC pin and channel 
    adc_pin_init(ADC1, GPIOA, PIN_7, ADC_CHANNEL_7, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_7, ADC_SEQ_2); 

#if ADC_DMA_THIRD_CHANNEL 

    // Initialize the third ADC pin and channel 
    adc_pin_init(ADC1, GPIOA, PIN_4, ADC_CHANNEL_4, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_4, ADC_SEQ_3); 

#endif   // ADC_DMA_THIRD_CHANNEL 

    // Set the sequence length (called once and only for more than one channel) 
    adc_seq_len_set(ADC1, (adc_seq_num_t)ADC_TEST_BUFF_SIZE); 

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
        DMA_DBM_DISABLE, 
#if DMA_TEST_MODE_3   // Memeory increment 
        DMA_ADDR_FIXED, 
#else 
        DMA_ADDR_INCREMENT, 
#endif 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)dma_data.adc_data, 
        (uint32_t)NULL, 
        (uint16_t)ADC_TEST_BUFF_SIZE); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 
    
    //==================================================

    //==================================================
    // Start and ADC conversion - ADC continuous modes 

#if (DMA_TEST_MODE_2 || DMA_TEST_MODE_3) 

    adc_start(ADC1); 

#endif 
    
    //==================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void dma_test_app()
{
    //==================================================
    // Trigger the ADC conversion - ADC non-continuous & scan mode only 

#if DMA_TEST_MODE_1 

    static uint8_t user_button = CLEAR; 
    static uint8_t button_block = CLEAR; 

    // Read the user button state 
    user_button = gpio_read(GPIOC, GPIOX_PIN_13); 

    // Start the ADC scan conversion through DMA if the button is pressed 
    if (!user_button && !button_block)
    {
        adc_start(ADC1); 
        button_block++; 
        tim_delay_ms(dma_data.timer, ADC_TEST_CONVERT_DELAY);  // Wait for button bounce to settle 
    }
    else if (user_button && button_block)
    {
        button_block--; 
    }

#endif   // DMA_TEST_MODE_1 

    //==================================================

    // Display the result to the serial terminal 
    uart_send_str(dma_data.uart, "First ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[FIRST_ADC]); 

#if ADC_DMA_SECOND_CHANNEL 

    uart_send_spaces(dma_data.uart, ADC_TEST_PRINT_SPACES); 
    uart_send_str(dma_data.uart, "Second ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[SECOND_ADC]); 

#if ADC_DMA_THIRD_CHANNEL 

    uart_send_spaces(dma_data.uart, ADC_TEST_PRINT_SPACES); 
    uart_send_str(dma_data.uart, "Third ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[THIRD_ADC]); 

#endif   // ADC_DMA_THIRD_CHANNEL 

    uart_send_new_line(dma_data.uart); 

#endif   // ADC_DMA_SECOND_CHANNEL 

    // Delay 
    tim_delay_ms(dma_data.timer, ADC_TEST_LOOP_DELAY); 
}

//=======================================================================================
