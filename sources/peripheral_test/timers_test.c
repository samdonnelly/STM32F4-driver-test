/**
 * @file timers_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Timers test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *            * LED connected to a GPIO output. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - Timers 
 *            * One timer configured for PWM output to control an LED. 
 *            * Another timer configured for periodic interrupts to control a counter 
 *              and the changing of the PWM duty cycle. 
 *            * A final general purpose timer used for non-blocking delays. 
 *          - Interrupts 
 *            * Interrupts are configured for the periodic counter timer. 
 *          - UART 
 *            * UART for serial terminal communication is configured to output the 
 *              value of the periodic counter and the non-blocking delay response. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * Timers, interrupts and UART drivers are used. 
 *          
 *          Procedure 
 *          - 3 different timers are configured to produce an output visible to the 
 *            user. The first is a PWM timer that controls an LED output. The second 
 *            is a periodic interrupt timer which triggers the output of a counter 
 *            to the serial terminal put is also used to update and change the PWM 
 *            duty cycle so the LED brightness changes. The final timer is a general 
 *            purpose timer used by the timers driver to produce a non-blocking delay 
 *            that will output a message periodically to the serial terminal. The purpose 
 *            of the last time is to demonstrate non-blocking delays without the use of 
 *            interrupts. These operations run continuously. 
 *          - When the code starts, it will output the value of the clock frequencies 
 *            being used. This information is used behind the scenes of the non-blocking
 *            delay to always produce a (somewhat) accurate delay regardless of the clock 
 *            frequency configured. 
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
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// PWM ouput 
#define TIM_TEST_PWM_INC 20
#define TIM_TEST_PWM_ARR 100 

// Non-blocking delay 
#define TIM_TEST_NO_BLOCK_DELAY 3000000   // (us) 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct tim_test_data_s
{
    USART_TypeDef *uart; 

    // PWM output 
    TIM_TypeDef *timer_pwm; 
    tim_channel_t tim_channel; 
    uint16_t pwm_value; 
    uint8_t pwm_dir; 

    // Periodic interrupt 
    uint8_t counter; 

    // Non-blocking delay 
    TIM_TypeDef *timer_delay; 
    tim_compare_t timer_delay_data; 
}
tim_test_data_t; 

static tim_test_data_t tim_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// Check system clock frequencies 
void tim_test_clk_freq(void); 

//=======================================================================================


//=======================================================================================
// Setup code

void timers_test_init()
{
    // Initialize GPIO ports 
    gpio_port_init(); 
    
    //===================================================
    // Initialize timers 

    // PWM output timer 
    tim_2_to_5_output_init(
        TIM3, 
        TIMER_CH1, 
        GPIOA,   // LED pin port 
        PIN_5,   // LED pin number 
        TIM_DIR_UP, 
        CLEAR, 
        TIM_TEST_PWM_ARR, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 
    tim_enable(TIM3); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x2710,   // ARR=10000, (10000 counts)*(100us/count) = 1s 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    // Initialize interrupt handler flags and enable the interrupt handlers 
    int_handler_init(); 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_0); 

    // Non-blocking delay timer 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0xFFFF,   // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM10); 
    
    //===================================================

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize data 
    tim_data.uart = USART2; 
    tim_data.timer_pwm = TIM3; 
    tim_data.tim_channel = TIMER_CH1; 
    tim_data.pwm_value = CLEAR; 
    tim_data.pwm_dir = SET_BIT; 
    tim_data.counter = CLEAR; 
    tim_data.timer_delay = TIM10; 
    tim_data.timer_delay_data.clk_freq = tim_get_pclk_freq(tim_data.timer_delay); 
    tim_data.timer_delay_data.time_cnt_total = CLEAR; 
    tim_data.timer_delay_data.time_cnt = CLEAR; 
    tim_data.timer_delay_data.time_start = SET_BIT; 

    // Check system clock frequencies 
    tim_test_clk_freq(); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void timers_test_app()
{
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        // Update the PWM output and calculate a new PWM value. 
        tim_ccr(tim_data.timer_pwm, tim_data.pwm_value, tim_data.tim_channel); 

        if (tim_data.pwm_dir)  // Counting up 
        {
            if (tim_data.pwm_value < TIM_TEST_PWM_ARR) 
            {
                tim_data.pwm_value += TIM_TEST_PWM_INC; 
            }
            else 
            {
                tim_data.pwm_dir = SET_BIT - tim_data.pwm_dir; 
                tim_data.pwm_value -= TIM_TEST_PWM_INC; 
            }
        }

        else  // Counting down 
        {
            if (tim_data.pwm_value > ZERO) 
            {
                tim_data.pwm_value -= TIM_TEST_PWM_INC; 
            }
            else 
            {
                tim_data.pwm_dir = SET_BIT - tim_data.pwm_dir; 
                tim_data.pwm_value += TIM_TEST_PWM_INC; 
            }
        }

        // Show the periodic interrupt printing a counter value to the serial terminal. 
        uart_send_integer(tim_data.uart, (int16_t)tim_data.counter++); 
        uart_send_new_line(tim_data.uart); 
    }

    
    // Non-blocking delay test 
    if (tim_compare(tim_data.timer_delay, 
                    tim_data.timer_delay_data.clk_freq, 
                    TIM_TEST_NO_BLOCK_DELAY, 
                    &tim_data.timer_delay_data.time_cnt_total, 
                    &tim_data.timer_delay_data.time_cnt, 
                    &tim_data.timer_delay_data.time_start))
    {
        // Print to the terminal to verify that the delay works 
        uart_send_str(tim_data.uart, "Delay!\r\n"); 
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// Check system clock frequencies 
void tim_test_clk_freq(void)
{
    uint32_t pclk1_frq = (rcc_get_pclk1_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t pclk2_frq = (rcc_get_pclk2_frq() / DIVIDE_1000) / DIVIDE_1000; 
    uint32_t hclk_frq  = (rcc_get_hclk_frq()  / DIVIDE_1000) / DIVIDE_1000; 

    uart_send_str(tim_data.uart, "PCLK1 Freq: "); 
    uart_send_integer(tim_data.uart, (int16_t)pclk1_frq); 
    uart_send_new_line(tim_data.uart); 
    uart_send_str(tim_data.uart, "PCLK2 Freq: "); 
    uart_send_integer(tim_data.uart, (int16_t)pclk2_frq); 
    uart_send_new_line(tim_data.uart); 
    uart_send_str(tim_data.uart, "HCLK Freq:  "); 
    uart_send_integer(tim_data.uart, (int16_t)hclk_frq); 
    uart_send_new_line(tim_data.uart); 
}

//=======================================================================================
