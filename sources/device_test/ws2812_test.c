/**
 * @file ws2812_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 (neopixels) test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller 
 *            * One or two strings of eight WS2812 LEDs connected to STM32F4 pins 
 *              configured for PWM output. 
 *          
 *          Configuration 
 *          - Timer 
 *            * A general purpose 1us counter timer that can be used for blocking delays. 
 *          - WS2812 driver 
 *            * WS2812 driver initialized with the timer and pins to use for the device. 
 *              The driver handles timer and pin configuration. 
 *            * If using a second device then the init function must be called again but 
 *              with a different device number. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * Timers and WS2812 LED drivers are used. 
 *          
 *          Procedure 
 *          - The first LED of the string is lit with a pre-defined colour. The index of 
 *            the LED is then incremented and the code delays a pre-defined amount of 
 *            time before clearing the first LED and lighting up the next LED with the 
 *            same colour. This is repeated continuously with the LED index resetting if 
 *            it goes out of range. This creates a moving LED pattern for the user to 
 *            see. 
 *          - A second LED can be added to demonstrate how to use the driver with 
 *            multiple devices. The operation is the same as the first device but the 
 *            colour is different. 
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
// Macros 

#define WS2812_TEST_DELAY_TIME 500 
#define WS2812_TEST_LED_OFF 0x000000 
#define WS2812_TEST_LED_ONE 0x001E1E 
#define WS2812_TEST_LED_TWO 0x2DAA00 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct ws2812_data_s
{
    TIM_TypeDef *timer; 
    uint8_t led_previous; 
    uint8_t led_current; 
    uint32_t s1_data[WS2812_LED_NUM];   // Colour order: Green, Red, Blue - 0x00GGRRBB 
#if WS2812_SECOND_DEVICE 
    uint32_t s2_data[WS2812_LED_NUM]; 
#endif   // WS2812_SECOND_DEVICE 
}
ws2812_data_t; 

static ws2812_data_t ws2812_data; 

//=======================================================================================


//=======================================================================================
// Test code 

// Setup code
void ws2812_test_init()
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // TIM9 - general purpose timer for delays 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 
    
    // Initialize WS2812 - device one 
    ws2812_init(
        DEVICE_ONE, 
        TIM3, 
        TIMER_CH1, 
        GPIOC, 
        PIN_6); 

    // Initialize data 
    ws2812_data.timer = TIM9; 
    ws2812_data.led_previous = WS2812_LED_7;   // Highest LED index 
    ws2812_data.led_current = WS2812_LED_0;    // Lowest LED index 
    memset((void *)ws2812_data.s1_data, CLEAR, sizeof(ws2812_data.s1_data)); 
    ws2812_send(DEVICE_ONE, ws2812_data.s1_data); 

#if WS2812_SECOND_DEVICE 

    // Initialize WS2812 - device two 
    ws2812_init(
        DEVICE_TWO, 
        TIM3, 
        TIMER_CH2, 
        GPIOC, 
        PIN_7); 

    // Initialize data 
    memset((void *)ws2812_data.s2_data, CLEAR, sizeof(ws2812_data.s2_data)); 
    ws2812_send(DEVICE_TWO, ws2812_data.s2_data); 

#endif   // WS2812_SECOND_DEVICE 
}


// Test code 
void ws2812_test_app()
{
    // Update the previous and current LED colour data then write it to device 1 
    ws2812_data.s1_data[ws2812_data.led_previous] = WS2812_TEST_LED_OFF; 
    ws2812_data.s1_data[ws2812_data.led_current] = WS2812_TEST_LED_ONE; 
    ws2812_send(DEVICE_ONE, ws2812_data.s1_data); 

#if WS2812_SECOND_DEVICE 

    // Update the previous and current LED colour data then write it to device 2 
    ws2812_data.s2_data[ws2812_data.led_previous] = WS2812_TEST_LED_OFF; 
    ws2812_data.s2_data[ws2812_data.led_current] = WS2812_TEST_LED_TWO; 
    ws2812_send(DEVICE_TWO, ws2812_data.s2_data); 

#endif   // WS2812_SECOND_DEVICE 

    // Update the LED index 
    if (++ws2812_data.led_previous >= WS2812_LED_NUM)
    {
        ws2812_data.led_previous = WS2812_LED_0; 
    }
    if (++ws2812_data.led_current >= WS2812_LED_NUM)
    {
        ws2812_data.led_current = WS2812_LED_0; 
    }

    // Delay for visual effect 
    tim_delay_ms(ws2812_data.timer, WS2812_TEST_DELAY_TIME); 
}

//=======================================================================================
