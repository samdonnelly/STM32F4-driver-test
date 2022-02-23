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
    static test_mode_t test_mode = SEND_STRING;
    static uint8_t input = 0;

    // Choose whoch test to run based on test_mode
    switch (test_mode)
    {
        // Send character 
        case SEND_CHAR:
            uart2_sendchar('G');
            HAL_Delay(1000);
            break;
        
        // Send string 
        case SEND_STRING:
            // TODO figure out why first character of string being sent over serial gets 
            // skipped. 
            uart2_sendstring("hello\r\n");
            HAL_Delay(1000);
            break;
        
        // Read character and resend 
        case GET_CHAR:
            // TODO set up enter key inputs in putty 
            input = uart2_getchar();
            uart2_sendchar(input);
            break;
        
        // Do nothing 
        default:
            break;
    }
}
