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

static uint32_t no_block_delay_count_total = CLEAR; 
static uint32_t no_block_delay_count_compare = CLEAR; 
static uint8_t  no_block_delay_start_flag = SET_BIT; 
static uint32_t no_block_delay_clk_freq; 

#if TIM_WS2812 

// LED colour data - Green, Red, Blue 
static uint8_t led_colour_data[WS2812_LED_NUM][WS2812_COLOUR_PER_LED]; 

#endif   // TIM_WS2812 

//=======================================================================================


// Setup code
void timers_test_init()
{
    // Setup code for the timers_test here 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 
    
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

    gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

#endif   // TIM_SWITCH_1 

#if TIM_SWITCH_2 

    gpio_pin_init(GPIOC, PIN_1, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

#endif   // TIM_SWITCH_2 

#if TIM_SWITCH_3 

    gpio_pin_init(GPIOC, PIN_2, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

#endif   // TIM_SWITCH_3 

#if TIM_SWITCH_4 

    gpio_pin_init(GPIOC, PIN_3, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

#endif   // TIM_SWITCH_4 

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
    memset((void *)led_colour_data[WS2812_LED_0], CLEAR, sizeof(led_colour_data)); 
    ws2812_send(DEVICE_ONE); 

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

    // Local variables 
    static uint16_t counter = 0; 
    static uint16_t pwm = 0; 
    static uint8_t pwm_dir = 1; 
    static uint8_t delay_status = CLEAR; 

    //===================================================
    // Periodic interrupt test 

#if TIM_PERIODIC 

    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        // Clear interrupt handler 
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        // Update the user button status 

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

        // Pole the GPIO input to get the button state 
        // Update the data record with the results 
        // Check the ellapsed time 

#if TIM_SWITCH_2 

#if TIM_SWITCH_3 

#if TIM_SWITCH_4 

#endif   // TIM_SWITCH_4 

#endif   // TIM_SWITCH_3 

#endif   // TIM_SWITCH_2 

#endif   // TIM_SWITCH_1 

#endif   // TIM_PERIODIC 

    //===================================================
    
    //===================================================
    // Non-blocking delay test 

#if TIM_NON_BLOCKING 

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
