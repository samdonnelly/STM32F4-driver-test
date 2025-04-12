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
 *            * Potentiometer connected to each of the enabled ADc channels. 
 *            * Push button if not using continuous conversion mode (see below). 
 *          - Software 
 *            * Serial monitor on a PC to allow the display of info from the STM32F4. 
 *          
 *          Configuration 
 *          - Timers 
 *            * General purpose timer configured for blocking delays. 
 *          - GPIO 
 *            * Push button input configured for non-continuous modes. 
 *          - UART 
 *            * One UART port must be configured for the serial terminal connection. 
 *          - ADC 
 *            * Up to 3 pins configured for ADC input. 
 *          - DMA 
 *            * DMA is configured for ADC data storage. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - Timers, GPIO, UART, ADC and DMA drivers are used. 
 *          
 *          Procedure 
 *          - This test will reac from ADC channels and display the results to a serial 
 *            terminal for the user to see. The user can choose how many channels are 
 *            enabled and which conversion modes to use. Details on each of these is 
 *            explained below. 
 *            
 *          - Number of devices: 
 *            * There can be up to 3 ADC conversions with a minimum of 1. The user can 
 *              choose how many they want. There needs to be a voltage device (such as 
 *              a potentiometer) hooked up for each ADC used. If more than one channel 
 *              is enabled but scan mode is not enabled, then each subsequent channel 
 *              conversion will overwrite the last in memory and the user will only 
 *              see the results from the most recent channel conversion (i.e. they 
 *              won't see each individual channel conversion). Scan mode must be 
 *              enabled if using multiple channels. 
 *          
 *          - Modes: 
 *            * 1. Non-continuous & non-scan (single read) mode 
 *              > In this mode the DMA does not perform ADC continuously in the 
 *                background and it reads from a single ADC channel. The conversion is 
 *                triggered manually by the user pressing a button and the button status 
 *                is checked periodically. Make sure to press and hold the button longer 
 *                than a period so it gets registered. 
 *            
 *            * 2. Non-continuous & scan mode 
 *              > In this mode the DMA does not perform ADC continuously in the 
 *                background but it will read from each enabled channel when a conversion 
 *                is triggered. The conversion is triggered manually by the user pressing 
 *                a button and the button status is checked periodically. Make sure to 
 *                press and hold the button longer than a period so it gets registered. 
 *              > Requires more than one channel to be enabled otherwise there won't be 
 *                any difference to mode 1. 
 *            
 *            * 3. Continuous & non-scan (single read) mode 
 *              > In this mode the DMA performs ADC continuously in the background and 
 *                it reads from a single ADC channel. The user button used for triggering 
 *                a conversion in modes 1 and 2 has no affect here. 
 *            
 *            * 4. Continuous & scan mode 
 *              > In this mode the DMA performs ADC continuously in the background and it 
 *                will read from each enabled channel during each conversion sequence. 
 *                The user button used for triggering a conversion in modes 1 and 2 has 
 *                no affect here. 
 *              > Requires more than one channel to be enabled otherwise there won't be 
 *                any difference to mode 3. 
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
// Globals 

typedef struct dma_test_data_s
{
    TIM_TypeDef *timer_delay; 
    USART_TypeDef *uart; 
    ADC_TypeDef *adc; 
    uint16_t adc_data[ADC_TEST_BUFF_SIZE]; 
    adc_seq_num_t seq_len; 

    // User button - only used when continuous mode is selected 
    GPIO_TypeDef *gpio_button; 
    pin_selector_t button_pin; 
    gpio_pin_num_t button_pin_num; 
    uint8_t button_status; 
    uint8_t button_block; 
}
dma_test_data_t; 

static dma_test_data_t dma_data; 

//=======================================================================================


//=======================================================================================
// Setup code

void dma_test_init()
{
    // Initialize data 
    dma_data.timer_delay = TIM9; 
    dma_data.uart = USART2; 
    dma_data.adc = ADC1; 
    memset((void *)dma_data.adc_data, CLEAR, sizeof(dma_data.adc_data)); 
    dma_data.seq_len = ADC_SEQ_1; 
    dma_data.gpio_button = GPIOC; 
    dma_data.button_pin = PIN_13; 
    dma_data.button_pin_num = SET_BIT << dma_data.button_pin; 
    dma_data.button_status = CLEAR; 
    dma_data.button_block = CLEAR; 

    // Initialize timers 
    tim_9_to_11_counter_init(
        dma_data.timer_delay, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(dma_data.timer_delay); 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        dma_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Configure a user push button to trigger ADC conversion. Only valid when not using 
    // continuous mode. This pin is commonly the user button on NUCLEO boards. 
    gpio_pin_init(
        dma_data.gpio_button, 
        dma_data.button_pin, 
        MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    //==================================================
    // ADC Init 

    // Initialize the ADC port 
    adc1_clock_enable(RCC); 
    adc_port_init(
        dma_data.adc, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
#if DMA_TEST_SCAN_MDOE           // ADC scan mode 
        ADC_PARAM_ENABLE, 
#else 
        ADC_PARAM_DISABLE, 
#endif 
#if DMA_TEST_CONT_MODE           // ADC continuous mode 
        ADC_PARAM_ENABLE, 
#else 
        ADC_PARAM_DISABLE, 
#endif 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize the first ADC pin and channel (called for each pin/channel), then set 
    // the channels place in the ADC conversion sequence (called for each sequence entry). 
    // More channels will be added if the user chooses to use more than one pin/channel. 
    // The length of the sequence is only set if more than one channel is used and it 
    // only has an affect when using scan mode. 
    adc_pin_init(dma_data.adc, GPIOA, PIN_6, ADC_CHANNEL_6, ADC_SMP_15); 
    adc_seq(dma_data.adc, ADC_CHANNEL_6, dma_data.seq_len); 

#if ADC_DMA_SECOND_CHANNEL 
    adc_pin_init(dma_data.adc, GPIOA, PIN_7, ADC_CHANNEL_7, ADC_SMP_15); 
    adc_seq(dma_data.adc, ADC_CHANNEL_7, ++dma_data.seq_len); 
#endif 

#if ADC_DMA_THIRD_CHANNEL 
    adc_pin_init(dma_data.adc, GPIOA, PIN_4, ADC_CHANNEL_4, ADC_SMP_15); 
    adc_seq(dma_data.adc, ADC_CHANNEL_4, ++dma_data.seq_len); 
#endif 

    adc_seq_len_set(dma_data.adc, dma_data.seq_len); 
    adc_on(dma_data.adc); 

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
#if DMA_TEST_SCAN_MDOE        // Memeory increment 
        DMA_ADDR_INCREMENT, 
#else 
        DMA_ADDR_FIXED, 
#endif 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&dma_data.adc->DR), 
        (uint32_t)dma_data.adc_data, 
        (uint32_t)NULL, 
        (uint16_t)dma_data.seq_len); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 
    
    //==================================================

#if DMA_TEST_CONT_MODE 
    // Start and ADC conversion for continuous mode 
    adc_start(dma_data.adc); 
#endif 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void dma_test_app()
{
    uint8_t adc_data_index = BYTE_0; 

#if !DMA_TEST_CONT_MODE 

    // Read the user button state. If the button is pressed after having not being 
    // previously pressed during the last check, then start the ADC scan conversion 
    // through DMA. 
    dma_data.button_status = gpio_read(dma_data.gpio_button, dma_data.button_pin_num); 

    if (!dma_data.button_status && !dma_data.button_block)
    {
        adc_start(dma_data.adc); 
        dma_data.button_block = SET_BIT; 
        tim_delay_ms(dma_data.timer_delay, ADC_TEST_CONVERT_DELAY);  // Wait for button bounce to settle 
    }
    else if (dma_data.button_status && dma_data.button_block)
    {
        dma_data.button_block = CLEAR_BIT; 
    }

#endif   // !DMA_TEST_CONT_MODE 

    // Display the result to the serial terminal. Show the number of channels defined 
    // by the user. After showing the results, delay shortly before repeating. 
    uart_send_str(dma_data.uart, "First ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[adc_data_index]); 

#if ADC_DMA_SECOND_CHANNEL 
    uart_send_spaces(dma_data.uart, ADC_TEST_PRINT_SPACES); 
    uart_send_str(dma_data.uart, "Second ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[++adc_data_index]); 
#endif 

#if ADC_DMA_THIRD_CHANNEL 
    uart_send_spaces(dma_data.uart, ADC_TEST_PRINT_SPACES); 
    uart_send_str(dma_data.uart, "Third ADC: "); 
    uart_send_integer(dma_data.uart, (int16_t)dma_data.adc_data[++adc_data_index]); 
#endif 

    uart_send_new_line(dma_data.uart); 
    tim_delay_ms(dma_data.timer_delay, ADC_TEST_LOOP_DELAY); 
}

//=======================================================================================
