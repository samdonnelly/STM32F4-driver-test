/**
 * @file user_app.h
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

#ifndef _USER_APP_H_
#define _USER_APP_H_

//=======================================================================================
// Includes 

#include "includes.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define LCD_NUM_LINES 4     // Number of lines on the LCD 
#define LCD_LINE_LENGTH 20  // Number of characters per line on LCD 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief LCD lines 
 * 
 */
typedef enum {
    LCD_L1,
    LCD_L2,
    LCD_L3,
    LCD_L4
} LCD_lines_t;

//=======================================================================================


//=======================================================================================
// Variables 

// Printable text 
// Repeatable text can be defined here for use throughout the code 
static char *hd44780u_test_text[LCD_NUM_LINES] = { "The",
                                                   "screen", 
                                                   "is", 
                                                   "working!" };

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief Contains the code that loops continuously. 
 * 
 */
void user_app(void);

//=======================================================================================


#endif  // _USER_APP_H_
