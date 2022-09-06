/**
 * @file m8q_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_test.h"

//=======================================================================================


//=======================================================================================
// Variables 

uint8_t nmea_msg[100]; 

//=======================================================================================


// Setup code
void m8q_test_init()
{
    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // Initialize I2C
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    // SAM-M8Q device setup 
    // Currently none 

    // Other setup 
    for (uint8_t i = 0; i < 100; i++)
    {
        nmea_msg[i] = 0; 
    }

    // Delay to let everything finish setup before starting to send and receieve data 
    tim9_delay_ms(500); 
} 


// Test code 
void m8q_test_app()
{
    // Local variables 
    uint16_t data_size = 0; 

    // Read the size of the NMEA data steam 
    m8q_read_nmea_ds(I2C1, &data_size); 

    // Print the data size 
    uart_send_integer(USART2, (int16_t)data_size); 
    uart_send_new_line(USART2); 

    // // Read available data 
    // if (m8q_read_nmea(I2C1, nmea_msg))
    // {
    //     uart_sendstring(USART2, (char *)nmea_msg); 
    // } 
    // else
    // {
    //     uart_send_new_line(USART2); 
    // }

    // Delay before starting over 
    tim9_delay_ms(1); 
}
