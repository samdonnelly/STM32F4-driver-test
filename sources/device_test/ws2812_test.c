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
static uint32_t s1_colour_data[WS2812_LED_NUM]; 

#if WS2812_SECOND_DEVICE 

// String 2 LED colour data - Green, Red, Blue 
static uint32_t s2_colour_data[WS2812_LED_NUM]; 

#endif   // WS2812_SECOND_DEVICE 

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
    
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 
    
    //==================================================

    //==================================================
    // Initialize devices 

    // WS2812 - device one 
    ws2812_init(
        DEVICE_ONE, 
        TIM3, 
        TIMER_CH1, 
        GPIOC, 
        PIN_6); 

#if WS2812_SECOND_DEVICE 

    // WS2812 - device two 
    ws2812_init(
        DEVICE_TWO, 
        TIM3, 
        TIMER_CH2, 
        GPIOC, 
        PIN_7); 

#endif   // WS2812_SECOND_DEVICE 
    
    //==================================================

    //==================================================
    // Initialize LED colours 

    // Clear colour data and turn off the LEDs 
    memset((void *)s1_colour_data, CLEAR, sizeof(s1_colour_data)); 
    ws2812_send(DEVICE_ONE, s1_colour_data); 

#if WS2812_SECOND_DEVICE 

    // Clear colour data and turn off the LEDs 
    memset((void *)s2_colour_data, CLEAR, sizeof(s1_colour_data)); 
    ws2812_send(DEVICE_TWO, s2_colour_data); 

#endif   // WS2812_SECOND_DEVICE 

    //==================================================
}


// Test code 
void ws2812_test_app()
{
    // Test code for the ws2812_test here 

    // Local variables 
    static uint8_t LED_previous = WS2812_LED_7; 
    static uint8_t LED_current = WS2812_LED_0; 

    // Update the previous LED colour data 
    s1_colour_data[LED_previous] = 0x000000; 

    // Update the current LED colour data 
    s1_colour_data[LED_current] = 0x001E1E; 

    // Write the LED data 
    ws2812_send(DEVICE_ONE, s1_colour_data); 

#if WS2812_SECOND_DEVICE 

    // Update the previous LED colour data 
    s2_colour_data[LED_previous] = 0x000000; 

    // Update the current LED colour data 
    s2_colour_data[LED_current] = 0x2DAA00; 

    // Write the LED data 
    ws2812_send(DEVICE_TWO, s2_colour_data); 

#endif   // WS2812_SECOND_DEVICE 

    // Update the LED index 
    LED_previous = (LED_previous >= WS2812_LED_7) ? (WS2812_LED_0) : (LED_previous + 1); 
    LED_current  = (LED_current >= WS2812_LED_7)  ? (WS2812_LED_0) : (LED_current + 1); 

    // Delay for visual effect 
    tim_delay_ms(TIM9, 500); 
}

//=======================================================================================
