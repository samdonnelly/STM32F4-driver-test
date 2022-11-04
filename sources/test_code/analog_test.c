/**
 * @file analog_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog test code 
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


// Setup code
void analog_test_init()
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
                  ADC_CONT_DISABLE, 
                  ADC_DMA_DISABLE, 
                  ADC_DDS_DISABLE); 

    // Initialize ADC pins and channels (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 
    adc_pin_init(ADC1, GPIOC, PIN_1, ADC_CHANNEL_11, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 
    adc_seq(ADC1, ADC_CHANNEL_11, ADC_SEQ_2); 

    // Set the sequence length (called once) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

    // Turn the ADC on 
    adc_on(ADC1); 

    //================================================== 
} 


// Test code 
void analog_test_app()
{
    // Test code for the analog_test here 

    // Turn the ADC on before trying to read 
    // Use a trigger to turn it on and off 

    // Local variables 
    uint16_t adc_result[2]; 

    // Read ADC - Single mode, no scan 
    // adc_result[0] = adc_read_single_select(ADC1, ADC_CHANNEL_10); 
    // adc_result[1] = adc_read_single_select(ADC1, ADC_CHANNEL_11); 

    // Read ADC - Single mode, with scan 
    adc_scan_seq(ADC1, ADC_SEQ_2, adc_result); 

    // // Turn to ADC off 
    // adc_off(ADC1); 

    // Display the result to the serial terminal 
    uart_sendstring(USART2, "ADC1_10: "); 
    uart_send_integer(USART2, (int16_t)adc_result[0]); 
    uart_send_spaces(USART2, 5); 
    uart_sendstring(USART2, "ADC1_11: "); 
    uart_send_integer(USART2, (int16_t)adc_result[1]); 
    uart_send_new_line(USART2); 

    // Delay 
    tim9_delay_ms(1000); 
}
