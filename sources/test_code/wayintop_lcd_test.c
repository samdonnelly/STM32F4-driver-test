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


//=======================================================================================
// Predefined text - can be defined here for use throughout the code 

static char* hd44780u_test_text[LCD_NUM_LINES] = 
{ 
    "The",
    "screen", 
    "is", 
    "working!" 
};

//=======================================================================================


// Setup code
void wayintop_lcd_test_init()
{
    // Setup code for the wayintop_lcd_test here 

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
    hd44780u_init();
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
    tim_delay_ms(TIM9, 1000);
}
