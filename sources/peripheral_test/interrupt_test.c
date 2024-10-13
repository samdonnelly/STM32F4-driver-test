/**
 * @file interrupt_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt test code 
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

// Conditional compilation 
#define INT_EXTI 1            // External interrupt code 
#define INT_INTERNAL 0        // Internal interrupt code 
#define INT_PERIODIC 0        // Periodic interrupt code 

//=======================================================================================


//=======================================================================================
// Prototypes 

// External interrupt code 
void int_test_external_init(void); 
void int_test_external_app(void); 

// Internal interrupt code 
void int_test_internal_init(void); 
void int_test_internal_app(void); 

// Periodic interrupt code 
void int_test_periodic_init(void); 
void int_test_periodic_app(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void int_test_init()
{
    // Interrupt initialization code goes here 

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
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    
    tim_enable(TIM9); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

#if INT_EXTI 
    int_test_external_init(); 
#elif INT_INTERNAL 
    int_test_internal_init(); 
#elif INT_PERIODIC 
    int_test_periodic_init(); 
#endif 
}

//=======================================================================================


//=======================================================================================
// Test code 

void int_test_app()
{
    // Test code for interrupt_test here 

#if INT_EXTI 
    int_test_external_app(); 
#elif INT_INTERNAL 
    int_test_internal_app(); 
#elif INT_PERIODIC 
    int_test_periodic_app(); 
#endif   // INT_EXTI 
}

//=======================================================================================


#if INT_EXTI 

// 

//=======================================================================================
// Macros 

// Conditional compilation 
#define INT_ADC_ENABLE 0      // ADC interrupt code (EXTI0 must be included as well) 
#define INT_DMA_ENABLE 0      // DMA interrupt code (EXTI0 & ADC must be included as well) 

// Data 
#define INT_ADC_NUM_CONV 2    // Number of ADC conversions to keep track of 

//=======================================================================================


//=======================================================================================
// Variables 

#if INT_ADC_ENABLE 

// Location for the DMA to store ADC values 
static uint16_t adc_conversion[INT_ADC_NUM_CONV]; 

#endif   // INT_ADC_ENABLE 

//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup 

// External interrupt test init 
void int_test_external_init(void)
{
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

    // Enable the interrupt handlers (called for each interrupt) 
    nvic_config(ADC_IRQn, EXTI_PRIORITY_1);           // ADC 
    nvic_config(DMA2_Stream0_IRQn, EXTI_PRIORITY_0);  // DMA2 Stream 0 

#endif   // INT_DMA_ENABLE 

#endif   // INT_ADC_ENABLE 
}

//=======================================================================================


//=======================================================================================
// Loop 

// External interrupt test app 
void int_test_external_app(void)
{
    // Check for the external interrupt from the user 
    if (handler_flags.exti0_flag)
    {
        // Reset the EXTI handler flag 
        handler_flags.exti0_flag = CLEAR; 

        // Do something to show the interrupt works 
        uart_sendstring(USART2, "got it!"); 
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
        uart_sendstring(USART2, "ADC1_10: "); 
        uart_send_integer(USART2, (int16_t)adc_conversion[0]); 
        uart_send_spaces(USART2, 5); 
        uart_sendstring(USART2, "ADC1_11: "); 
        uart_send_integer(USART2, (int16_t)adc_conversion[1]); 
        uart_send_new_line(USART2); 

#endif   // INT_ADC_ENABLE 
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================


#elif INT_INTERNAL 

// 

//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Variables 
//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================

//=======================================================================================
// Setup 

// Internal interrupt test init 
void int_test_internal_init(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Loop 

// Internal interrupt test app 
void int_test_internal_app(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================


#elif INT_PERIODIC 

// 

//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Variables 
//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================

//=======================================================================================
// Setup 

// Periodic interrupt test init 
void int_test_periodic_init(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Loop 

// Periodic interrupt test app 
void int_test_periodic_app(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================

#endif 
