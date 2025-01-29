/**
 * @file interrupt_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * 
 *          - Software 
 *            * 
 *          
 *          Configuration 
 *          - Timers 
 *            * 
 *          - Interrupts 
 *            * 
 *          - UART 
 *            * 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * 
 *          
 *          Procedure 
 *          - 
 *          
 *          Other tests demonstrating the interrupt driver: 
 *          - esc_readytosky_test.c 
 *          - wheel_rpm_test.c 
 *          - circular_buffer_test.cpp 
 * 
 * @version 0.1
 * @date 2022-11-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "interrupt_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define INT_ADC_NUM_CONV 2    // Number of ADC conversions to keep track of 

//=======================================================================================


//=======================================================================================
// Global variables 

#if INT_ADC_ENABLE 

// Location for the DMA to store ADC values 
static uint16_t adc_conversion[INT_ADC_NUM_CONV]; 

#endif   // INT_ADC_ENABLE 

//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup code 

void int_test_init()
{
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
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 


    //==================================================
    // External interrupt initialization 

    // Enable external interrupts (called once) 
    exti_init(); 

    // The external interrupt was tested by associating it with a hall effect sensor 
    // that would change state in the presence of a magnetic field. 

    exti_config(
        GPIOB, 
        EXTI_PB, 
        PIN_0, 
        PUPDR_PU, 
        EXTI_L0, 
        EXTI_INT_NOT_MASKED, 
        EXTI_EVENT_MASKED, 
        EXTI_RISE_TRIG_DISABLE, 
        EXTI_FALL_TRIG_ENABLE); 

    // Enable the interrupt handlers (called for each interrupt) 
    nvic_config(EXTI0_IRQn, EXTI_PRIORITY_0); 

    //==================================================

#if INT_ADC_ENABLE 

    //==================================================
    // ADC 

    // Initialize the ADC port (called once) 
    adc1_clock_enable(RCC); 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize ADC pins and channels (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 
    adc_pin_init(ADC1, GPIOC, PIN_1, ADC_CHANNEL_11, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 
    adc_seq(ADC1, ADC_CHANNEL_11, ADC_SEQ_2); 

    // Set the sequence length (called once) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

    // Turn the ADC on 
    adc_on(ADC1); 

    //================================================== 

#if INT_DMA_ENABLE 

    //==================================================
    // DMA 

    // Initialize the DMA stream 
    dma_stream_init(
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_conversion, 
        (uint16_t)SET_2); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 

    // Configure the DMA interrupts 
    dma_int_config(
        DMA2_Stream0, 
        DMA_TCIE_ENABLE, 
        DMA_HTIE_DISABLE, 
        DMA_TEIE_DISABLE, 
        DMA_DMEIE_DISABLE); 
    
    //==================================================


#endif   // INT_DMA_ENABLE 

#endif   // INT_ADC_ENABLE 

    //==================================================
    // Interrups 

    // Enable the interrupt handlers (called for each interrupt) 
    nvic_config(ADC_IRQn, EXTI_PRIORITY_1);           // ADC 
    nvic_config(DMA2_Stream0_IRQn, EXTI_PRIORITY_0);  // DMA2 Stream 0 
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void int_test_app()
{
    // Check for the external interrupt from the user 
    if (handler_flags.exti0_flag)
    {
        // Reset the EXTI handler flag 
        handler_flags.exti0_flag = CLEAR; 

        // Do something to show the interrupt works 
        uart_send_str(USART2, "got it!"); 
        uart_send_new_line(USART2); 

#if INT_ADC_ENABLE 

        // Start the ADC conversions 
        adc_start(ADC1); 

        // Wait for the ADC sequence to be converted 
        while(!(handler_flags.adc_flag)); 

        // Reset the ADC handler flag 
        handler_flags.adc_flag = CLEAR; 

#if INT_DMA_ENABLE 

        // Wait for the DMA transfer to complete 
        while(!(handler_flags.dma2_0_flag)); 

        // Reset the DMA handler flag 
        handler_flags.dma2_0_flag = CLEAR; 

#endif   // INT_DMA_ENABLE 

        // Display the ADC results to the serial terminal 
        uart_send_str(USART2, "ADC1_10: "); 
        uart_send_integer(USART2, (int16_t)adc_conversion[0]); 
        uart_send_spaces(USART2, 5); 
        uart_send_str(USART2, "ADC1_11: "); 
        uart_send_integer(USART2, (int16_t)adc_conversion[1]); 
        uart_send_new_line(USART2); 

#endif   // INT_ADC_ENABLE 
    }
}

//=======================================================================================


//=======================================================================================
// Interrupt handlers 

// These override the weak handler definitions in "stm32f4xx_it.c". When this test is not 
// in use INTERRUPT_OVERRIDE should be set to zero so other tests don't use this version 
// of the handler. 

#if INTERRUPT_OVERRIDE 

// EXTI Line 0 
void EXTI0_IRQHandler(void)
{
    handler_flags.exti0_flag = SET_BIT; 
    exti_pr_clear(EXTI_L0); 
}


// Timer 1 break + timer 9 global 
void TIM1_BRK_TIM9_IRQHandler(void)
{
    handler_flags.tim1_brk_tim9_glbl_flag = SET_BIT; 
    tim_uif_clear(TIM1); 
    tim_uif_clear(TIM9); 
}


// ADC1 
void ADC_IRQHandler(void)
{
    handler_flags.adc_flag = SET_BIT;  
}

#endif   // INTERRUPT_OVERRIDE 

//=======================================================================================
