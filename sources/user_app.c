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
    // Local variables 
    static int8_t counter = 0;

    // Print each line one at a time followed by a delay 
    switch (counter)
    {
        // Text is cast to a char pointer for use in the send_string function because 
        // its declaration in the header defaults to an int type. 

        case LCD_L1:
            hd44780u_send_instruc(HD44780U_START_L1);
            hd44780u_send_string((char *)(hd44780u_test_text[LCD_L1]));
            break;
        
        case LCD_L2:
            hd44780u_send_instruc(HD44780U_START_L2);
            hd44780u_send_string((char *)(hd44780u_test_text[LCD_L2]));
            break;
        
        case LCD_L3:
            hd44780u_send_instruc(HD44780U_START_L3);
            hd44780u_send_string((char *)(hd44780u_test_text[LCD_L3]));
            break;
        
        case LCD_L4:
            hd44780u_send_instruc(HD44780U_START_L4);
            hd44780u_send_string((char *)(hd44780u_test_text[LCD_L4]));
            break;

        default:
            hd44780u_clear();
            counter = -1;
            break;
    }

    // Increment to next line
    counter++;

    // Delay for 1 second 
    tim9_delay_ms(1000);
}
