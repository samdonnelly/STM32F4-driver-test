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

// uint8_t nmea_msg[150];  // Store NMEA messages 

//=======================================================================================


// Setup code
void m8q_test_init()
{
    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize GPIO ports 
    // TODO remove this step from other comm and device inits 
    gpio_port_init(); 

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
    uint16_t init_error_code = m8q_init(I2C1, 
                                        GPIOC, 
                                        PIN_10, 
                                        PIN_11, 
                                        M8Q_CONFIG_MSG_NUM, 
                                        M8Q_CONFIG_MSG_MAX_LEN, 
                                        (uint8_t *)m8q_config_messages[0]); 
    if (init_error_code) uart_sendstring(USART2, "M8Q init fault.\r\n"); 
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
    //===================================================
    // Data record, power save mode and TX-Ready testing 

    // Local variables 
    uint8_t counter = 0; 
    static uint8_t flipper = 0; 
    static uint16_t timer = 0x48B8; 
    static uint8_t blink = 0; 

    //  M8Q data 
    uint16_t lat_deg_min = 0; 
    uint32_t lat_min_frac = 0; 
    volatile uint8_t NS = 0; 
    uint16_t lon_deg_min = 0; 
    uint32_t lon_min_frac = 0; 
    volatile uint8_t EW = 0; 
    volatile uint16_t navstat = 0; 
    uint8_t utc_time[BYTE_9]; 
    uint8_t utc_date[BYTE_6]; 

    while (TRUE)
    {
        if (m8q_get_tx_ready())
        {
            // Read the data 
            m8q_read(); 
            counter++; 

            // Blink the board LED for visual feedback 
            blink = GPIO_HIGH - blink; 
            gpio_write(GPIOA, GPIOX_PIN_5, blink); 
        }
        else
        {
            if (counter == 2)
            {
                m8q_get_lat(&lat_deg_min, &lat_min_frac); 
                NS = m8q_get_NS(); 
                m8q_get_long(&lon_deg_min, &lon_min_frac); 
                EW = m8q_get_EW(); 
                navstat = m8q_get_navstat(); 
                m8q_get_time(utc_time); 
                m8q_get_date(utc_date); 
                counter = 0; 
            }
            break; 
        }
    }

    // Toggle the EXTINT pin 
    if (!(--timer))
    {
        m8q_set_low_power(flipper); 
        flipper = GPIO_HIGH - flipper; 
        timer = 0x68B8; 
        tim9_delay_ms(150);  // Give time for the receiver to startup from sleep mode 

        // The following line of code was needed in order for the TX_READY input pin to start 
        // functioning normally again after low power mode. Make this part of a state when 
        // returning form low power mode.  
        if (!flipper) while (!(m8q_read())); 
    }

    tim9_delay_ms(1);

    //===================================================

#endif   // M8Q_USER_CONFIG
}
