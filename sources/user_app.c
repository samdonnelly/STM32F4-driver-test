/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
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


// User application 
void user_app()
{
    // This code is continuously looped

    // Local variables 
    static test_mode_t test_mode = GET_STR;
    static uint8_t input = 0;
    // static char string_read[STR_READ_LEN] = {'\0'};
    static char string_read[STR_READ_LEN];

    // Choose which test to run based on test_mode
    switch (test_mode)
    {
        // Send character 
        case SEND_CHAR:
            uart2_sendchar('G');
            HAL_Delay(1000);
            break;
        
        // Send string 
        case SEND_STRING:
            uart2_sendstring("hello world!\r\n");
            HAL_Delay(1000);
            break;
        
        // Read character from serial and resend 
        case GET_CHAR:
            // Check if data is waiting to be read
            if (USART2->SR & (SET_BIT << SHIFT_5))
            {
                input = uart2_getchar();
                uart2_sendchar(input);
                uart2_sendstring("\r\n");
            }

            break;
        
        // Read string from serial and resend 
        case GET_STR:
            // Check if data is waiting to be read
            if (USART2->SR & (SET_BIT << SHIFT_5))
            {
                // Pass a pointer to cleared string to uart2_getstr()
                uart2_getstr(string_read);
                uart2_sendstring(string_read);
                uart2_sendstring("\r\n");

                // input = *string_read;

                // // Relay read string back over serial to verify contents 
                // uart2_sendstring(string_read);

                // Clear previous contents of string
                // for (uint8_t i = 0; i < STR_READ_LEN; i++)
                // {
                //     string_read[i] = '\0';
                // }
            }

            break;
        
        // Do nothing 
        default:
            break;
    }
}
