/**
 * @file gpio_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPIO test code 
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
// Setup code

void gpio_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize GPIO pins 
    gpio_pin_init(GPIOB, PIN_0,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOB, PIN_1,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOB, PIN_8,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOB, PIN_9,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOB, PIN_10, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOC, PIN_0,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOC, PIN_1,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOC, PIN_2,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOC, PIN_3,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_pin_init(GPIOC, PIN_6,  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    // Set the initial state of the GPIO pins 
    gpio_write(GPIOB, GPIOX_PIN_0,  GPIO_LOW); 
    gpio_write(GPIOB, GPIOX_PIN_1,  GPIO_LOW); 
    gpio_write(GPIOB, GPIOX_PIN_8,  GPIO_HIGH); 
    gpio_write(GPIOB, GPIOX_PIN_9,  GPIO_LOW); 
    gpio_write(GPIOB, GPIOX_PIN_10, GPIO_LOW); 
    gpio_write(GPIOC, GPIOX_PIN_0,  GPIO_LOW); 
    gpio_write(GPIOC, GPIOX_PIN_1,  GPIO_LOW); 
    gpio_write(GPIOC, GPIOX_PIN_2,  GPIO_LOW); 
    gpio_write(GPIOC, GPIOX_PIN_3,  GPIO_LOW); 
    gpio_write(GPIOC, GPIOX_PIN_6,  GPIO_LOW); 

} 

//=======================================================================================


//=======================================================================================
// Test code 

void gpio_test_app(void)
{
    // Use this space to set and observe pin states 
}

//=======================================================================================
