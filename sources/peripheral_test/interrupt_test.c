/**
 * @file interrupt_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * Two potentiometers to provide ADC input to the STM32F4. 
 *            * One button for the user to press. 
 *          - Software 
 *            * Serial monitor on a PC to allow the display of info from the STM32F4. 
 *          
 *          Configuration 
 *          - Timers 
 *            * One timer configured to be a periodic interrupt timer. 
 *          - UART 
 *            * UART configured to output to the serial terminal to show the user what's 
 *              happening. 
 *          - ADC 
 *            * Two ADCs configured for input. 
 *          - DMA 
 *            * DMA configured to transfer ADC data to memory. 
 *          - Interrupts 
 *            * Periodic timer interrupt. 
 *            * External interrupt from user button - handles GPIO input pull up config. 
 *            * ADC sequence complete interrupt. 
 *            * DMA transfer complete interrupt. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * Timers, UART, ADC, DMA and interrupt drivers used. 
 *          
 *          Procedure 
 *          - There are four interrupts configured that control what the code does. The 
 *            first is a periodic interrupt that will increment and display a counter 
 *            in the serial terminal once per second. The second is an external interrupt 
 *            which gets triggered when the user presses a button connected to the 
 *            STM32F4. The external interrupt will display a message to the serial 
 *            terminal and start an ADC conversion of the two ADCs connected. The 
 *            remaining two interrupts are an ADC sequence complete interrupt and a 
 *            DMA transfer complete interrupt which will trigger in succession after 
 *            the button press. 
 *          - The interrupt code is contained in the interrupt handlers below. These 
 *            handlers override the weak definition of each handler and requires the 
 *            INTERRUPT_OVERRIDE macro to be set to use them. 
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

#define INT_TEST_NUM_ADC 2    // Number of ADC conversions to keep track of 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct int_test_data_s
{
    USART_TypeDef *uart; 
    ADC_TypeDef *adc; 
    uint16_t adc_data[INT_TEST_NUM_ADC]; 
    uint8_t timer_counter; 
}
int_test_data_t; 

static int_test_data_t int_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup code 

void int_test_init()
{
    // Initialize data 
    int_data.uart = USART2; 
    int_data.adc = ADC1; 
    memset((void *)int_data.adc_data, CLEAR, sizeof(int_data.adc_data)); 
    int_data.timer_counter = CLEAR; 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        int_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x2710,   // ARR=10000, (10000 counts)*(100us/count) = 1s 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    //==================================================
    // ADC 

    // Initialize the ADC port 
    adc1_clock_enable(RCC); 
    adc_port_init(
        int_data.adc, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_DISABLE,    // EOC bit set after the sequence is converted 
        ADC_PARAM_ENABLE,     // EOC interrupt enabled 
        ADC_PARAM_ENABLE,     // Scan mode enabled 
        ADC_PARAM_DISABLE,    // Continuous mode disabled - conversion controlled by user 
        ADC_PARAM_ENABLE,     // DMA enabled 
        ADC_PARAM_ENABLE,     // Continue to use DMA as long as there is a request 
        ADC_PARAM_DISABLE);   // Overrun interrupt disabled 

    // Initialize ADC pins and channels, set each channel sequence order and set sequence 
    // length before turning on the ADC. 
    adc_pin_init(int_data.adc, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 
    adc_pin_init(int_data.adc, GPIOC, PIN_1, ADC_CHANNEL_11, ADC_SMP_15); 
    adc_seq(int_data.adc, ADC_CHANNEL_10, ADC_SEQ_1); 
    adc_seq(int_data.adc, ADC_CHANNEL_11, ADC_SEQ_2); 
    adc_seq_len_set(int_data.adc, ADC_SEQ_2); 

    adc_on(int_data.adc); 

    //================================================== 

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
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&int_data.adc->DR), 
        (uint32_t)int_data.adc_data, 
        (uint32_t)NULL, 
        (uint16_t)INT_TEST_NUM_ADC); 

    // Configure the DMA interrupts 
    dma_int_config(
        DMA2_Stream0, 
        DMA_TCIE_ENABLE, 
        DMA_HTIE_DISABLE, 
        DMA_TEIE_DISABLE, 
        DMA_DMEIE_DISABLE); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 
    
    //==================================================

    //==================================================
    // Interrups 

    // Initialize interrupt handler flags (called once) 
    // This is only used when using the interrupt handler flags. This test does not use 
    // them but it's added here for reference. Other tests that use interrupts will 
    // likely use the flags. The flags are useful to identify when interrupts have 
    // happened but the action doesn't need to be immediately addressed or the action 
    // is larger than what should be put into a handler. 
    int_handler_init(); 

    // Enable external interrupts (called once if using external interrupts) 
    exti_init(); 

    // EXTI0 - user button 
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
    nvic_config(EXTI0_IRQn, EXTI_PRIORITY_0);           // EXTI0 - user button 
    nvic_config(ADC_IRQn, EXTI_PRIORITY_1);             // ADC1 - ADC conversion 
    nvic_config(DMA2_Stream0_IRQn, EXTI_PRIORITY_2);    // DMA2 Stream 0 - ADC conversion 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_3);   // TIM9 - periodic timer 
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void int_test_app()
{
    // Everything from this test is handled in the interrupt handlers so there is no 
    // test code here. 
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
    exti_pr_clear(EXTI_L0); 

    // External interrupt. Triggered when the user presses the button. This then triggers 
    // the ADC which has its own interrupt when it's done converting. 
    uart_send_str(int_data.uart, "Pressed!\r\n"); 
    adc_start(int_data.adc); 
}


// DMA2 Stream 0 
void DMA2_Stream0_IRQHandler(void)
{
    dma_clear_int_flags(DMA2); 

    // DMA transfer complete interrupt. The DMA transfer gets triggered when the ADC 
    // sequence finishes. ADC data is now stored in memory and we can display it for 
    // the user to see. 
    uart_send_str(int_data.uart, "Stored!\r\n"); 
    uart_send_str(int_data.uart, "ADC1_10: "); 
    uart_send_integer(int_data.uart, (int16_t)int_data.adc_data[0]); 
    uart_send_new_line(int_data.uart); 
    uart_send_str(int_data.uart, "ADC1_11: "); 
    uart_send_integer(int_data.uart, (int16_t)int_data.adc_data[1]); 
    uart_send_new_line(int_data.uart); 
}


// Timer 1 break + timer 9 global 
void TIM1_BRK_TIM9_IRQHandler(void)
{
    tim_uif_clear(TIM1); 
    tim_uif_clear(TIM9); 

    // Periodic interrupt. Output a count to the serial terminal. 
    int_data.timer_counter++; 
    uart_send_integer(int_data.uart, (int16_t)int_data.timer_counter); 
    uart_send_new_line(int_data.uart); 
}


// ADC1 
void ADC_IRQHandler(void)
{
    // ADC conversion sequence complete interrupt. The ADC conversion gets triggered by 
    // a user button press (external interrupt). After the conversion finishes the DMA 
    // will move the ADC data to memory and trigger it's own interrupt. 
    uart_send_str(int_data.uart, "Converted!\r\n"); 
}

#endif   // INTERRUPT_OVERRIDE 

//=======================================================================================
