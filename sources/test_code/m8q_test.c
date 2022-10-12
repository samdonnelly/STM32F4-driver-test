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
    // TODO make the GPIO switching into a setter in the driver 

    //===================================================
    // TX_READY testing 

    // Local variables 
    // static uint8_t identifier = 0; 
    volatile uint8_t in_pin_stat = 0; 
    // static uint8_t blocker = 1; 

    // Check the pin 
    in_pin_stat = gpio_read(GPIOA, GPIOX_PIN_11); 

    if (in_pin_stat)
    {
        m8q_read(I2C1, nmea_msg); 
        uart_sendstring(USART2, (char *)nmea_msg); 
        uart_send_new_line(USART2); 
    }

    // if (blocker && in_pin_stat)
    // {
    //     uart_sendstring(USART2, "Input pin status: 1\r\n"); 
    //     blocker = 0; 
    // }
    // else if (!blocker && !in_pin_stat)
    // {
    //     uart_sendstring(USART2, "Input pin status: 0\r\n"); 
    //     blocker = 1; 
    // }

    // // Check if data is available 
    // if (in_pin_stat)
    // {
    //     uart_sendstring(USART2, "Input pin status: "); 
    //     uart_send_integer(USART2, (int16_t)in_pin_stat); 
    //     uart_send_new_line(USART2); 

    //     if (m8q_read(I2C1, nmea_msg))
    //     {
    //         if (identifier)
    //             uart_sendstring(USART2, "++There is data!\r\n"); 
    //         else 
    //             uart_sendstring(USART2, "--There is data!\r\n"); 
            
    //         identifier = 1 - identifier; 
    //     }
    // }

    // // Clear the data 
    // while (m8q_read(I2C1, nmea_msg)); 

    //===================================================

    //===================================================
    // Low Power mode testing
    
    // // Local variables 
    // static uint8_t flipper = 0; 
    // static uint16_t timer = 0x88B8; 

    // // Display the message if it exists 
    // if (!flipper)
    // {
    //     while (m8q_read(I2C1, nmea_msg))    
    //     {
    //         uart_sendstring(USART2, (char *)nmea_msg); 
    //         uart_send_new_line(USART2); 
    //     }
    // }
    
    // // Toggle the EXTINT pin 
    // if (!(--timer))
    // {
    //     gpio_write(GPIOA, GPIOX_PIN_8, flipper);
    //     flipper = 1 - flipper; 
    //     timer = 0x88B8; 
    //     tim9_delay_ms(100); 
    // }

    // tim9_delay_ms(1); 

    //===================================================


    //===================================================
    // Data record testing 

    // // Local variables 
    // uint8_t counter = 0; 
    // //  M8Q data 
    // uint16_t lat_deg_min = 0; 
    // uint32_t lat_min_frac = 0; 
    // volatile uint8_t NS = 0; 
    // uint16_t lon_deg_min = 0; 
    // uint32_t lon_min_frac = 0; 
    // volatile uint8_t EW = 0; 
    // volatile uint16_t navstat = 0; 
    // uint8_t utc_time[BYTE_9]; 
    // uint8_t utc_date[BYTE_6]; 

    // while (TRUE)
    // {
    //     if (m8q_read(I2C1, nmea_msg))
    //         counter++; 
    //     else
    //     {
    //         if (counter == 2)
    //         {
    //             m8q_get_lat(&lat_deg_min, &lat_min_frac); 
    //             NS = m8q_get_NS(); 
    //             m8q_get_long(&lon_deg_min, &lon_min_frac); 
    //             EW = m8q_get_EW(); 
    //             navstat = m8q_get_navstat(); 
    //             m8q_get_time(utc_time); 
    //             m8q_get_date(utc_date); 
    //             counter = 0; 
    //         }
    //         break; 
    //     }
    // }

    //===================================================

#endif   // M8Q_USER_CONFIG
}
