/**
 * @file timers_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Timers test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "timers_test.h"

//=======================================================================================


//=======================================================================================
// Global variables 

#if TIM_NON_BLOCKING 

static uint32_t no_block_delay_count_total = CLEAR; 
static uint32_t no_block_delay_count_compare = CLEAR; 
static uint8_t  no_block_delay_start_flag = SET_BIT; 
static uint32_t no_block_delay_clk_freq; 

#endif   // TIM_NON_BLOCKING 

#if TIM_WS2812 

// LED colour data - Green, Red, Blue 
static uint32_t led_colour_data[WS2812_LED_NUM]; 

#endif   // TIM_WS2812 

//=======================================================================================


// Setup code
void timers_test_init()
{
    // Setup code for the timers_test here 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_TX_DISABLE, 
        UART_DMA_RX_DISABLE); 
    
    //===================================================
    // Initialize timers 

#if TIM_PERIODIC 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x0032,  // ARR=50, (50 counts)*(100us/count) = 5ms 
        TIM_UP_INT_ENABLE); 

    tim_enable(TIM9); 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_0); 

#endif   // TIM_PERIODIC 

#if TIM_PWM_OUTPUT 

    // PWM output timer 
    tim_2_to_5_output_init(
        TIM3, 
        TIM_CHANNEL_1, 
        GPIOC, 
        PIN_6, 
        TIM_DIR_UP, 
        CLEAR, 
        PWM_ARR, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 

    tim_enable(TIM3); 

#endif   // TIM_PWM_OUTPUT 

#if TIM_NON_BLOCKING 

    // Non-blocking delay timer 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0x2710,  // ARR=10000, (10000 counts)*(100us/count) = 1s 
        TIM_UP_INT_DISABLE); 

    tim_enable(TIM10); 

#endif   // TIM_NON_BLOCKING 
    
    //===================================================

    //===================================================
    // Initialize GPIO inputs (user buttons/switches) 

#if TIM_SWITCH_1 

    uint8_t pull_mask = CLEAR; 

    gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    pull_mask |= GPIOX_PIN_0; 

#if TIM_SWITCH_2 

    gpio_pin_init(GPIOC, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    pull_mask |= GPIOX_PIN_1; 

#if TIM_SWITCH_3 

    gpio_pin_init(GPIOC, PIN_2, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    pull_mask |= GPIOX_PIN_2; 

#if TIM_SWITCH_4 

    gpio_pin_init(GPIOC, PIN_3, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 
    pull_mask |= GPIOX_PIN_3; 

#endif   // TIM_SWITCH_4 

#endif   // TIM_SWITCH_3 

#endif   // TIM_SWITCH_2 

    // Initialize the button debouncer 
    debounce_init(pull_mask); 

#endif   // TIM_SWITCH_1 

    //===================================================

    //===================================================
    // Initialize WS2812 

#if TIM_WS2812 

    ws2812_init(
        DEVICE_ONE, 
        TIM3, 
        TIM_CHANNEL_1, 
        GPIOC, 
        PIN_6); 

    // Clear colour data and turn off the LEDs 
    memset((void *)led_colour_data, CLEAR, sizeof(led_colour_data)); 
    ws2812_send(DEVICE_ONE, led_colour_data); 

#endif   // TIM_WS2812 
    
    //===================================================

    //===================================================
    // Check system clock frequencies 

#if TIM_CLK_FREQ 

    uint32_t pclk1_frq = (rcc_get_pclk1_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t pclk2_frq = (rcc_get_pclk2_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t hclk_frq  = (rcc_get_hclk_frq()  / DIVIDE_1000) / DIVIDE_1000; 

    uart_sendstring(USART2, "PCLK1 Freq: "); 
    uart_send_integer(USART2, (int16_t)pclk1_frq); 
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "PCLK2 Freq: "); 
    uart_send_integer(USART2, (int16_t)pclk2_frq); 
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "HCLK Freq:  "); 
    uart_send_integer(USART2, (int16_t)hclk_frq); 
    uart_send_new_line(USART2); 

    no_block_delay_clk_freq = tim_get_pclk_freq(TIM9); 

#endif   // TIM_CLK_FREQ 

    //===================================================
} 


// Test code 
void timers_test_app()
{
    // Test code for the timers_test here 

    //===================================================
    // Local variables 

#if TIM_PERIODIC 

#if TIM_SWITCH_1 

    static uint8_t button_block_1 = CLEAR; 

#endif   // TIM_SWITCH_1 

#if TIM_SWITCH_2 

    static uint8_t button_block_2 = CLEAR; 

#endif   // TIM_SWITCH_2 

#if TIM_SWITCH_3 

    static uint8_t button_block_3 = CLEAR; 

#endif   // TIM_SWITCH_3 

#if TIM_SWITCH_4 

    static uint8_t button_block_4 = CLEAR; 

#endif   // TIM_SWITCH_4 

#endif   // TIM_PERIODIC 

#if TIM_PERIODIC_COUNT 

    static uint16_t counter = 0; 

#endif   // TIM_PERIODIC_COUNT 

#if TIM_PWM_OUTPUT 

    static uint16_t pwm = 0; 
    static uint8_t pwm_dir = 1; 

#endif   // TIM_PWM_OUTPUT 

#if TIM_NON_BLOCKING 

    static uint8_t delay_status = CLEAR; 

#endif   // TIM_NON_BLOCKING 

    //===================================================

    //===================================================
    // Periodic interrupt test 

#if TIM_PERIODIC 

    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        // Clear interrupt handler 
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        // Update the user button status 
        debounce((uint8_t)gpio_port_read(GPIOC)); 

#if TIM_PWM_OUTPUT   // PWM output test 

        // Set PWM output and calculate a new value 
        tim_ccr(TIM3, pwm, TIM_CHANNEL_1); 

        if (pwm_dir)  // Counting up 
        {
            if (pwm < PWM_ARR) 
            {
                pwm += PWM_INCREMENT; 
            }
            else 
            {
                pwm_dir = 1 - pwm_dir; 
                pwm -= PWM_INCREMENT; 
            }
        }

        else  // Counting down 
        {
            if (pwm > NONE) 
            {
                pwm -= PWM_INCREMENT; 
            }
            else 
            {
                pwm_dir = 1 - pwm_dir; 
                pwm += PWM_INCREMENT; 
            }
        }

#endif   // TIM_PWM_OUTPUT 

#if TIM_PERIODIC_COUNT 

        // Print the counter to the terminal 
        uart_sendstring(USART2, "Counter: "); 
        uart_send_integer(USART2, (int16_t)counter); 
        uart_send_new_line(USART2); 
        counter++; 

#endif   // TIM_PERIODIC_COUNT 

    }

#if TIM_SWITCH_1 

    // Check if the button is pressed 
    if (debounce_pressed((uint8_t)GPIOX_PIN_0) && !button_block_1) 
    {
        // Toggle the LED 
        led_colour_data[WS2812_LED_0] = 0x001E1E - led_colour_data[WS2812_LED_0]; 
        ws2812_send(DEVICE_ONE, led_colour_data); 
        button_block_1 = SET_BIT; 
    }
    // Check if the button is released  
    else if (debounce_released((uint8_t)GPIOX_PIN_0) && button_block_1) 
    {
        button_block_1 = CLEAR; 
    }

#if TIM_SWITCH_2 

    // Check if the button is pressed 
    if (debounce_pressed((uint8_t)GPIOX_PIN_1) && !button_block_2) 
    {
        // Toggle the LED 
        led_colour_data[WS2812_LED_0] = 0x1E1E00 - led_colour_data[WS2812_LED_0]; 
        ws2812_send(DEVICE_ONE, led_colour_data); 
        button_block_2 = SET_BIT; 
    }
    // Check if the button is released  
    else if (debounce_released((uint8_t)GPIOX_PIN_1) && button_block_2) 
    {
        button_block_2 = CLEAR; 
    }

#if TIM_SWITCH_3 

    // Check if the button is pressed 
    if (debounce_pressed((uint8_t)GPIOX_PIN_2) && !button_block_3) 
    {
        // Toggle the LED 
        led_colour_data[WS2812_LED_0] = 0x1E001E - led_colour_data[WS2812_LED_0]; 
        ws2812_send(DEVICE_ONE, led_colour_data); 
        button_block_3 = SET_BIT; 
    }
    // Check if the button is released  
    else if (debounce_released((uint8_t)GPIOX_PIN_2) && button_block_3) 
    {
        button_block_3 = CLEAR; 
    }

#if TIM_SWITCH_4 

    // Check if the button is pressed 
    if (debounce_pressed((uint8_t)GPIOX_PIN_3) && !button_block_4) 
    {
        // Toggle the LED 
        led_colour_data[WS2812_LED_0] = 0x1E1E1E - led_colour_data[WS2812_LED_0]; 
        ws2812_send(DEVICE_ONE, led_colour_data); 
        button_block_4 = SET_BIT; 
    }
    // Check if the button is released  
    else if (debounce_released((uint8_t)GPIOX_PIN_3) && button_block_4) 
    {
        button_block_4 = CLEAR; 
    }

#endif   // TIM_SWITCH_4 

#endif   // TIM_SWITCH_3 

#endif   // TIM_SWITCH_2 

#endif   // TIM_SWITCH_1 

#endif   // TIM_PERIODIC 

    //===================================================
    
    //===================================================
    // Non-blocking delay test 

#if TIM_NON_BLOCKING 

    // Local variables 
    static uint8_t delay_status = CLEAR; 

    delay_status = tim_compare(TIM10, 
                               no_block_delay_clk_freq, 
                               TIM_NO_BLOCK_DELAY, 
                               &no_block_delay_count_total, 
                               &no_block_delay_count_compare, 
                               &no_block_delay_start_flag); 

    if (delay_status)
    {
        // Print to the terminal to verify that the delay works 
        uart_sendstring(USART2, "Delay!"); 
        uart_send_new_line(USART2);

        // Reset the start bit 
        no_block_delay_start_flag = SET_BIT; 
    }

    // Do stuff to show that the delay works 

#endif   // TIM_NON_BLOCKING 

    //===================================================
}
