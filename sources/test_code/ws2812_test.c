/**
 * @file ws2812_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 (neopixels) test code 
 * 
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "ws2812_test.h"

//=======================================================================================


//=======================================================================================
// Global variables 

// String 1 LED colour data - Green, Red, Blue 
static uint8_t s1_colour_data[WS2812_LED_NUM][WS2812_COLOUR_PER_LED] = 
{
    {30, 0, 0},          // LED1 starting colour 
    {30, 30, 0},         // LED2 starting colour 
    {0, 30, 0},          // LED3 starting colour 
    {0, 30, 30},         // LED4 starting colour 
    {0, 0, 30},          // LED5 starting colour 
    {0, 0, 0},           // LED6 starting colour 
    {0, 0, 0},           // LED7 starting colour 
    {0, 30, 0}           // LED8 starting colour 
};

// String 1 LED colour data - Green, Red, Blue 
static uint8_t s2_colour_data[WS2812_LED_NUM][WS2812_COLOUR_PER_LED] = 
{
    {0, 0, 30},          // LED1 starting colour 
    {0, 30, 30},         // LED2 starting colour 
    {0, 30, 0},          // LED3 starting colour 
    {30, 30, 0},         // LED4 starting colour 
    {30, 0, 0},          // LED5 starting colour 
    {0, 0, 0},           // LED6 starting colour 
    {0, 0, 0},           // LED7 starting colour 
    {0, 30, 0}           // LED8 starting colour 
};

//=======================================================================================


//=======================================================================================
// Test code 

// Setup code
void ws2812_test_init()
{
    // Setup code for the ws2812_test here 

    //==================================================
    // Peripheral initialization 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // TIM9 - general purpose timer for delays 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 

    // TIM10 - timer for WS2812 data sending 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_310NS_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 

    // Enable the timers 
    tim_enable(TIM9); 
    tim_enable(TIM10); 

    // Initialize UART2
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);
    
    //==================================================

    //==================================================
    // Initialize devices 

    // WS2812 - device one 
    ws2812_init(
        DEVICE_ONE, 
        TIM10, 
        GPIOC, 
        PIN_6); 

#if WS2812_SECOND_DEVICE 

    // WS2812 - device two 
    ws2812_init(
        DEVICE_TWO, 
        TIM10, 
        GPIOC, 
        PIN_7); 

#endif   // WS2812_SECOND_DEVICE 
    
    //==================================================

    //==================================================
    // Initialize LED colours 

    // Set each LED colour 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_0], WS2812_LED_0); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_1], WS2812_LED_1); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_2], WS2812_LED_2); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_3], WS2812_LED_3); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_4], WS2812_LED_4); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_5], WS2812_LED_5); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_6], WS2812_LED_6); 
    ws2812_colour_set(DEVICE_ONE, s1_colour_data[WS2812_LED_7], WS2812_LED_7); 

    // Update the write data with the new LED colour infromation 
    ws2812_update(DEVICE_ONE); 

    // Send the write data to the device 
    ws2812_send(DEVICE_ONE); 

#if WS2812_SECOND_DEVICE 

    // Set each LED colour 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_0], WS2812_LED_0); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_1], WS2812_LED_1); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_2], WS2812_LED_2); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_3], WS2812_LED_3); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_4], WS2812_LED_4); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_5], WS2812_LED_5); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_6], WS2812_LED_6); 
    ws2812_colour_set(DEVICE_TWO, s2_colour_data[WS2812_LED_7], WS2812_LED_7); 

    // Update the write data with the new LED colour infromation 
    ws2812_update(DEVICE_TWO); 

    // Send the write data to the device 
    ws2812_send(DEVICE_TWO); 

#endif   // WS2812_SECOND_DEVICE 

    //==================================================
}


// Test code 
void ws2812_test_app()
{
    // Test code for the ws2812_test here 

    // Delay at least 50us 
    tim_delay_us(TIM9, 50); 
}

//=======================================================================================
