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
    // Delay 
    tim9_delay_ms(1000);

    // Test print 
    char string[] = "Hello";
    hd44780u_send_string(string);
}
