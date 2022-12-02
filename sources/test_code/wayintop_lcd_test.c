/**
 * @file wayintop_lcd_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief wayintop LCD test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "wayintop_lcd_test.h"

//=======================================================================================


//===============================================================================
// Notes 
// - The screen is really sensitive to timing. Adding delays between write operations 
//   seems to be more reliable. 
//===============================================================================


//=======================================================================================
// Predefined text - can be defined here for use throughout the code 

static char* hd44780u_startup_screen[LCD_NUM_LINES] = 
{ 
    "Get",
    "ready", 
    "to", 
    "rumpus!" 
};


static char* hd44780u_test_text[LCD_NUM_LINES] = 
{ 
    "Rump",
    "till", 
    "you", 
    "drop!" 
};

//=======================================================================================


// Setup code
void wayintop_lcd_test_init()
{
    // Setup code for the wayintop_lcd_test here 

    //=================================================
    // Peripheral initialization 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // I2C1 init
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // wayintop LCD screen init. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH);

    //=================================================

    //=================================================
    // Screen/UI initialization - Move to the contoller when it's made 

    // Screen startup message - Use in controller init state 
    hd44780u_line_set(
        HD44780U_L1, 
        (char *)(hd44780u_startup_screen[HD44780U_L1]), 
        HD44780U_CURSOR_OFFSET_3); 
    hd44780u_line_set(
        HD44780U_L2, 
        (char *)(hd44780u_startup_screen[HD44780U_L2]), 
        HD44780U_CURSOR_OFFSET_0); 
    hd44780u_line_set(
        HD44780U_L3, 
        (char *)(hd44780u_startup_screen[HD44780U_L3]), 
        HD44780U_CURSOR_OFFSET_6); 
    hd44780u_line_set(
        HD44780U_L4, 
        (char *)(hd44780u_startup_screen[HD44780U_L4]), 
        HD44780U_CURSOR_OFFSET_7); 

    // Send all lines of data 
    hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L1); 

    hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L2); 
    
    hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L3); 
    
    hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L4); 

    // Give time for the startup message to display then clear the message 
    tim_delay_ms(TIM9, 2000); 
    hd44780u_clear();
    tim_delay_ms(TIM9, 500);  // Adding this delay helps the screen transition to test_app 

    //=================================================
} 


// Test code 
void wayintop_lcd_test_app()
{
    // Test code for the wayintop_lcd_test here 

    // Local variables 
    static int8_t counter = 0;

    // Print each line one at a time followed by a delay 
    switch (counter)
    {
        // Text is cast to a char pointer for use in the send_string function because 
        // its declaration in the header defaults to an int type. 

        case HD44780U_L1:
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_10);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L1]));
            break;
        
        case HD44780U_L2:
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_8);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L2])); 
            break;
        
        case HD44780U_L3:
            hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_6);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L3])); 
            break;
        
        case HD44780U_L4:
            hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_4);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L4])); 
            break;

        default:
            hd44780u_clear();
            counter = -1;
            break;
    }

    // Increment to next line
    counter++;

    // Delay for 1 second 
    tim_delay_ms(TIM9, 1000);
}
