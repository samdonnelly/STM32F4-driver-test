/**
 * @file hd44780u_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U LCD screen test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "hd44780u_test.h"

//=======================================================================================


//=======================================================================================
// Global variables 

static char* hd44780u_startup_screen[HD44780U_NUM_LINES] = 
{ 
    "Get",
    "ready", 
    "to", 
    "rumpus!" 
};

static char* hd44780u_test_text[HD44780U_NUM_LINES] = 
{ 
    "Rump",
    "till", 
    "you", 
    "drop!" 
};

//================================================================================


//=======================================================================================
// Setup code

void hd44780u_test_init()
{
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

    // Initialize UART - Serial terminal comms 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // I2C1 init
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // LCD screen init 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH);

    //=================================================

    //=================================================
    // Screen/UI initialization - Move to the contoller when it's made 

    // Screen startup message - Use in controller init state 
    hd44780u_line_set(
        HD44780U_L1, 
        (char *)(hd44780u_startup_screen[HD44780U_L1]), 
        3); // Random offset 
    hd44780u_line_set(
        HD44780U_L2, 
        (char *)(hd44780u_startup_screen[HD44780U_L2]), 
        HD44780U_CURSOR_NO_OFFSET); 
    hd44780u_line_set(
        HD44780U_L3, 
        (char *)(hd44780u_startup_screen[HD44780U_L3]), 
        6); // Random offset 
    hd44780u_line_set(
        HD44780U_L4, 
        (char *)(hd44780u_startup_screen[HD44780U_L4]), 
        7); // Random offset 

    // Send all lines of data 
    hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
    hd44780u_send_line(HD44780U_L1); 

    hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
    hd44780u_send_line(HD44780U_L2); 
    
    hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_NO_OFFSET);
    hd44780u_send_line(HD44780U_L3); 
    
    hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_NO_OFFSET);
    hd44780u_send_line(HD44780U_L4); 

    // Give time for the startup message to display then clear the message 
    tim_delay_ms(TIM9, 2000); 
    hd44780u_clear();
    tim_delay_ms(TIM9, 500);  // Adding this delay helps the screen transition to test_app 

    //=================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void hd44780u_test_app()
{
    static int8_t counter = 0; 

#if HD44780U_BACKLIGHT_TEST 
    static uint8_t backlight = 1; 
#endif   // HD44780U_BACKLIGHT_TEST 

#if HD44780U_DISPLAY_TEST 
    static uint8_t display = 1; 
#endif   // HD44780U_DISPLAY_TEST 

#if HD44780U_CURSOR_TEST 
    static uint8_t cursor = 1; 
#endif   // HD44780U_CURSOR_TEST 

#if HD44780U_BLINK_TEST 
    static uint8_t blink = 1; 
#endif   // HD44780U_BLINK_TEST 

    // Print each line one at a time followed by a delay 
    switch (counter)
    {
        // Text is cast to a char pointer for use in the send_string function because 
        // its declaration in the header defaults to an int type. 

        case HD44780U_L1:
            hd44780u_cursor_pos(HD44780U_START_L1, 10);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L1]));
            break;
        
        case HD44780U_L2:
            hd44780u_cursor_pos(HD44780U_START_L2, 8);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L2])); 
            break;
        
        case HD44780U_L3:
            hd44780u_cursor_pos(HD44780U_START_L3, 6);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L3])); 
            break;
        
        case HD44780U_L4:
            hd44780u_cursor_pos(HD44780U_START_L4, 4);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L4])); 
            break;

        default:
            hd44780u_clear();
            counter = -1; 

#if HD44780U_BACKLIGHT_TEST 
            // Backlight test 
            if (backlight) hd44780u_backlight_off(); 
            else hd44780u_backlight_on(); 
            backlight = 1 - backlight; 
#endif   // HD44780U_BACKLIGHT_TEST 

#if HD44780U_DISPLAY_TEST 
            // Display test 
            if (display) hd44780u_display_off(); 
            else hd44780u_display_on(); 
            display = 1 - display; 
#endif   // HD44780U_DISPLAY_TEST 

#if HD44780U_CURSOR_TEST 
            // Cursor test 
            if (cursor) hd44780u_cursor_on(); 
            else hd44780u_cursor_off(); 
            cursor = 1 - cursor; 
#endif   // HD44780U_CURSOR_TEST 

#if HD44780U_BLINK_TEST 
            // Cursor blink test 
            if (blink) hd44780u_blink_on(); 
            else hd44780u_blink_off(); 
            blink = 1 - blink; 
#endif   // HD44780U_BLINK_TEST 

            break;
    }

    // Increment to next screen line
    counter++;

    // Delay for 1 second 
    tim_delay_ms(TIM9, 1000);

    //==================================================
}

//=======================================================================================
