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

    // Initialize the ADC port 
    adc_port_init(ADC_PCLK2_4, ADC_RES_10, ADC_EOC_EACH); 

    // Initialize ADC pins and channels 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 

    // Set the ADC conversion sequence 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 

    // Set the sequence length 
    adc_seq_len_set(ADC1, ADC_SEQ_1); 

    //==================================================
} 


// Test code 
void analog_test_app()
{
    // Test code for the analog_test here 

    // Turn the ADC on before trying to read 
    // Use a trigger to turn it on and off 

    // Local variables 
    uint16_t adc_result = 0; 

    // Turn the ADC on 
    adc_on(ADC1); 

    tim9_delay_ms(100); 

    // Start and ADC conversion 
    adc_start(ADC1); 

    // Wait for start bit to set 
    adc_start_wait(ADC1); 

    // Wait for end of ADC conversion 
    adc_eoc_wait(ADC1); 

    // Read the data register 
    adc_result = adc_dr(ADC1); 

    // Turn to ADC off 
    adc_off(ADC1); 

    // Display the result to the serial terminal 
    uart_sendstring(USART2, "ADC Result: "); 
    uart_send_integer(USART2, (int16_t)adc_result); 
    uart_send_new_line(USART2); 

    // Delay 
    tim9_delay_ms(1000); 
}
