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

uint8_t nmea_msg[150];  // Store NMEA messages 

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

    // M8Q configuration mode 
#if M8Q_USER_CONFIG 
    m8q_nmea_config_ui(); 
#else
    // M8Q device setup 
    char m8q_config_messages[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN]; 
    m8q_config_copy(m8q_config_messages); 
    m8q_init(I2C1, 
             M8Q_CONFIG_MSG_NUM, 
             M8Q_CONFIG_MSG_MAX_LEN, 
             (uint8_t *)m8q_config_messages[0]); 
#endif

    // Delay to let everything finish setup before starting to send and receieve data 
    tim9_delay_ms(500); 
} 


// Test code 
void m8q_test_app()
{
#if M8Q_USER_CONFIG 
    m8q_user_config(I2C1); 
#else 
    // Local variables 
    // uint16_t data_size = 0; 
    uint8_t counter = 0; 
    int16_t lat_deg_min = 0; 
    int32_t lat_min_frac = 0; 
    // volatile float latitude = 0; 

    // Read the size of the NMEA data stream 
    // m8q_check_data_size(I2C1, &data_size); 

    // 
    while (TRUE)
    {
        // 
        if (m8q_read(I2C1, nmea_msg))
        {
            counter++; 
        }
        else
        {
            if (counter == 2)
            {
                m8q_get_lat(&lat_deg_min, &lat_min_frac); 
                // uart_sendstring(USART2, "latitude: "); 
                // uart_send_integer(USART2, (int16_t)counter); 
                // uart_send_new_line(USART2); 
                counter = 0; 
            }
            break; 
        }
    }

#endif   // M8Q_USER_CONFIG
}
