/**
 * @file analog_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *          
 *          Configuration 
 *          - UART 
 *            * A UART port is configured for a serial terminal connection which 
 *              allows the display of analog data for the user to see. 
 *          - ADC 
 *            * Two analog pins are configured for ADC. 
 *          - Timers 
 *            * A general pupose timer is configured to provide blocking delays between 
 *              ADC reads. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - 
 *          
 *          Procedure 
 *          - 
 *          
 *          Other tests demonstrating the analog driver: 
 *          - dma_test.c 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "analog_test.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define ANALOG_TEST_NUM_ADC 2 
#define ANALOG_TEST_DELAY_TIME 1000 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct analog_test_data_s 
{
    USART_TypeDef *uart; 
    ADC_TypeDef *adc; 
    TIM_TypeDef *timer; 
    uint16_t adc_data[ANALOG_TEST_NUM_ADC]; 
    adc_channel_t adc_channel[ANALOG_TEST_NUM_ADC]; 

}
analog_test_data_t; 

static analog_test_data_t analog_data; 

//=======================================================================================


//=======================================================================================
// Setup code

void analog_test_init()
{
    // Initialize data 
    analog_data.uart = USART2; 
    analog_data.adc = ADC1; 
    analog_data.timer = TIM9; 
    memset((void *)analog_data.adc_data, CLEAR, sizeof(analog_data.adc_data)); 
    analog_data.adc_channel[0] = ADC_CHANNEL_10; 
    analog_data.adc_channel[1] = ADC_CHANNEL_11; 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        analog_data.timer, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(analog_data.timer); 

    // Initialize UART
    uart_init(
        analog_data.uart, 
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
        analog_data.adc, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize ADC pins and channels (called for each pin/channel) 
    adc_pin_init(analog_data.adc, GPIOC, PIN_0, analog_data.adc_channel[0], ADC_SMP_15); 
    adc_pin_init(analog_data.adc, GPIOC, PIN_1, analog_data.adc_channel[1], ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    // This only has an effect when scan mode is enabled 
    adc_seq(analog_data.adc, analog_data.adc_channel[0], ADC_SEQ_1); 
    adc_seq(analog_data.adc, analog_data.adc_channel[1], ADC_SEQ_2); 

    // Set the sequence length (called once) 
    // This only has an effect when scan mode is enabled 
    adc_seq_len_set(analog_data.adc, ADC_SEQ_2); 

    // Turn the ADC on 
    adc_on(analog_data.adc); 

    //================================================== 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void analog_test_app()
{
#if ADC_MODE_0 

    // Read ADC - Single read mode, scan mode disabled 
    analog_data.adc_data[0] = adc_read_single(analog_data.adc, analog_data.adc_channel[0]); 
    analog_data.adc_data[1] = adc_read_single(analog_data.adc, analog_data.adc_channel[1]); 

#elif ADC_MODE_1 

    // Read ADC - Single read mode, scan mode enabled 
    adc_scan_seq(analog_data.adc, ADC_SEQ_2, analog_data.adc_data); 

#endif 

    // Display the result to the serial terminal 
    uart_send_str(analog_data.uart, "ADC1_10: "); 
    uart_send_integer(analog_data.uart, (int16_t)analog_data.adc_data[0]); 
    uart_send_spaces(analog_data.uart, 5); 
    uart_send_str(analog_data.uart, "ADC1_11: "); 
    uart_send_integer(analog_data.uart, (int16_t)analog_data.adc_data[1]); 
    uart_send_new_line(analog_data.uart); 

    // Delay 
    tim_delay_ms(analog_data.timer, ANALOG_TEST_DELAY_TIME); 
}

//=======================================================================================
