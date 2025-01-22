/**
 * @file switch_debounce_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Switch debounce test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller 
 *            * Physical switches that can toggle a GPIO input (pull-up or pull-down) 
 *              wired to GPIO pins on the STM32F4. 
 *            * LED connected to a GPIO output. 
 *          
 *          Configuration 
 *          - Timer 
 *            * 
 *          - GPIO 
 *            * 
 *          - Switch debounce 
 *            * 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * Timers, GPIO and switch debouncing drivers are used. 
 *          
 *          Procedure 
 *          - 
 * 
 * @version 0.1
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "switch_debounce_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define SD_TEST_NUM_SWITCH 2 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct sd_data_s 
{
    // Switches 
    GPIO_TypeDef *gpio_switch; 
    gpio_pin_num_t switch_pin[SD_TEST_NUM_SWITCH]; 
    uint8_t pull_mask; 
    uint8_t button_block[SD_TEST_NUM_SWITCH]; 

    // LED 
    GPIO_TypeDef *gpio_led; 
    gpio_pin_num_t led_pin; 
}
sd_data_t; 

static sd_data_t sd_data; 

//=======================================================================================


//=======================================================================================
// Setup code

void switch_debounce_test_init()
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x0032,  // ARR=50, (50 counts)*(100us/count) = 5ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    // Initialize interrupt handler flags and enable the interrupt handlers 
    int_handler_init(); 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_0); 

    // Initialize variables 
    sd_data.gpio_switch = GPIOC; 
    sd_data.switch_pin[PIN_0] = GPIOX_PIN_0; 
    sd_data.switch_pin[PIN_1] = GPIOX_PIN_1; 
    sd_data.pull_mask = CLEAR; 
    sd_data.button_block[PIN_0] = CLEAR; 
    sd_data.button_block[PIN_1] = CLEAR; 
    sd_data.gpio_led = GPIOA; 
    sd_data.led_pin = GPIOX_PIN_5; 

    // Initialize GPIO inputs (user buttons/switches) 
    gpio_pin_init(sd_data.gpio_switch, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    sd_data.pull_mask |= sd_data.switch_pin[PIN_0]; 
    gpio_pin_init(sd_data.gpio_switch, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    sd_data.pull_mask |= sd_data.switch_pin[PIN_1]; 

    // Configure an LED to toggle with the switches 
    gpio_pin_init(sd_data.gpio_led, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(sd_data.gpio_led, sd_data.led_pin, GPIO_LOW); 

    // Initialize the button debouncer 
    debounce_init(sd_data.pull_mask); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void switch_debounce_test_app()
{
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        // Update the user button status and check if any buttons are newly pressed or 
        // released. If so then perform the needed action (LED toggle on or off) and 
        // block the action from being triggered again until the button status has 
        // changed. 

        debounce((uint8_t)gpio_port_read(sd_data.gpio_switch)); 

        if (debounce_pressed((uint8_t)sd_data.switch_pin[PIN_0]) && 
            !sd_data.button_block[PIN_0]) 
        {
            sd_data.button_block[PIN_0] = SET_BIT; 
            gpio_write(sd_data.gpio_led, sd_data.led_pin, GPIO_HIGH); 
        }
        else if (debounce_released((uint8_t)sd_data.switch_pin[PIN_0]) && 
                 sd_data.button_block[PIN_0]) 
        {
            sd_data.button_block[PIN_0] = CLEAR; 
        }

        if (debounce_pressed((uint8_t)sd_data.switch_pin[PIN_1]) && 
            !sd_data.button_block[PIN_1]) 
        {
            sd_data.button_block[PIN_1] = SET_BIT; 
            gpio_write(sd_data.gpio_led, sd_data.led_pin, GPIO_LOW); 
        }
        else if (debounce_released((uint8_t)sd_data.switch_pin[PIN_1]) && 
                 sd_data.button_block[PIN_1]) 
        {
            sd_data.button_block[PIN_1] = CLEAR; 
        }
    }
}

//=======================================================================================
