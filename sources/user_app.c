/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Tests the uart_comm.c driver 
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_app.h"

//=======================================================================================


// User application - Run every loop 
void user_app()
{
    // Local variables 
    static test_mode_t test_mode = GET_STR;

    // Choose which test to run based on test_mode
    switch (test_mode)
    {
        case SEND_CHAR:
            send_character();
            break;
        
        case SEND_STRING:
            send_string();
            break;
        
        case GET_CHAR:
            get_character();
            break;
        
        case GET_STR:
            get_string();
            break;
        
        default:
            break;
    }
}

// Send a single character to the serial terminal 
void send_character(void)
{
    // Repeatedly send a character using uart2_sendchar()
    uart2_sendchar('G');

    // Delay 1 second 
    HAL_Delay(1000);
}

// Send a string to the serial terminal 
void send_string(void)
{
    // Repeatedly send a string using uart2_sendstring()
    uart2_sendstring("hello world!\r\n");

    // Delay 1 second
    HAL_Delay(1000);
}

// Get a single character from the serial terminal 
void get_character(void)
{
    // To record the character from the serial terminal 
    static uint8_t input = 0;

    // Check if data is waiting to be read
    if (USART2->SR & (SET_BIT << SHIFT_5))
    {
        input = uart2_getchar();

        // Resend information to serial terminal 
        uart2_sendchar(input);
        uart2_sendstring("\r\n");
    }
}

// Get a string from the serial terminal 
void get_string(void)
{
    // To record the string from the serial terminal 
    static char string_read[STR_READ_LEN];

    // Check if data is waiting to be read
    if (USART2->SR & (SET_BIT << SHIFT_5))
    {
        uart2_getstr(string_read);

        // Resend information to serial terminal
        uart2_sendstring(string_read);
        uart2_sendchar(*("\n"));
    }
}
