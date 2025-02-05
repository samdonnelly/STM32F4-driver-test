/**
 * @file gpio_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPIO test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller. 
 *            * LED connected to an STM32F4 output pin. 
 *            * Push button connected to an STM32F4 input pin. 
 *          
 *          Configuration 
 *          - GPIO 
 *            * Push button input configured for non-continuous modes. 
 *          - Timers 
 *            * A general purpose timer is configured for blocking delays. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - Timers, GPIO, UART, ADC and DMA drivers are used. 
 *          
 *          Procedure 
 *          - When the user presses a push button it will toggle that state of an LED. 
 * 
 * @version 0.1
 * @date 2023-09-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "gpio_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define GPIO_TEST_DELAY 10   // (ms) 

//=======================================================================================


//=======================================================================================
// Global data 

typedef struct gpio_test_data_s 
{
    TIM_TypeDef *timer_delay; 

    // User button 
    GPIO_TypeDef *gpio_button; 
    pin_selector_t button_pin; 
    gpio_pin_num_t button_pin_num; 
    uint8_t button_status; 
    uint8_t button_block; 

    // LED 
    GPIO_TypeDef *gpio_led; 
    pin_selector_t led_pin; 
    gpio_pin_num_t led_pin_num; 
    gpio_pin_state_t led_state; 
}
gpio_test_data_t; 

static gpio_test_data_t gpio_data; 

//=======================================================================================


//=======================================================================================
// Setup code

void gpio_test_init(void)
{
    // Initialize data 
    gpio_data.timer_delay = TIM9; 
    gpio_data.gpio_button = GPIOC; 
    gpio_data.button_pin = PIN_13; 
    gpio_data.button_pin_num = SET_BIT << gpio_data.button_pin; 
    gpio_data.button_status = CLEAR; 
    gpio_data.button_block = CLEAR; 
    gpio_data.gpio_led = GPIOA; 
    gpio_data.led_pin = PIN_5; 
    gpio_data.led_pin_num = SET_BIT << gpio_data.led_pin; 
    gpio_data.led_state = GPIO_LOW; 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        gpio_data.timer_delay, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(gpio_data.timer_delay); 

    // Configure a user push button to toggle the LED. This pin is commonly the user 
    // button on NUCLEO boards. 
    gpio_pin_init(
        gpio_data.gpio_button, 
        gpio_data.button_pin, 
        MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    // Configure an LED to show the results of using the push button. This pin is 
    // commonly the NUCLEO board LED. 
    gpio_pin_init(
        gpio_data.gpio_led, 
        gpio_data.led_pin, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(gpio_data.gpio_led, gpio_data.led_pin_num, gpio_data.led_state); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void gpio_test_app(void)
{
    // Check the user button state. If the button is pressed after having not been 
    // then the LED state will be toggled. The state of the LED cannot be changed again 
    // until the button is release. There is a short delay to account for button bounce. 
    gpio_data.button_status = gpio_read(gpio_data.gpio_button, gpio_data.button_pin_num); 

    if (!gpio_data.button_status && !gpio_data.button_block)
    {
        gpio_data.button_block = SET_BIT; 
        gpio_data.led_state = GPIO_HIGH - gpio_data.led_state; 
        gpio_write(gpio_data.gpio_led, gpio_data.led_pin_num, gpio_data.led_state); 
    }
    else if (gpio_data.button_status && gpio_data.button_block)
    {
        gpio_data.button_block = CLEAR_BIT; 
    }

    tim_delay_ms(gpio_data.timer_delay, GPIO_TEST_DELAY); 
}

//=======================================================================================
