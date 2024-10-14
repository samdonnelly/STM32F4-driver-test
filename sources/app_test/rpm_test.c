/**
 * @file rpm_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief RPM (revolutions per minute) test 
 * 
 * @version 0.1
 * @date 2024-10-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "rpm_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define RPM_SAMPLE_BUFF_SIZE 4    // Number of samples for RPM calculation 
#define RPM_OUTPUT_BUFF_SIZE 10   // Output string buffer size 
#define PRM_SAMPLE_PERIOD 0.5     // Time between samples (seconds) 
#define RPM_SEC_TO_MIN 60         // 60 seconds / minute 

//=======================================================================================


//=======================================================================================
// Global data 

typedef struct rpm_test_data_s 
{
    // Wheel revolution data 
    uint8_t rev_count;                          // Revolution counter 
    uint8_t rev_buff_index;                     // Revolution circular buffer index 
    uint8_t rev_buff[RPM_SAMPLE_BUFF_SIZE];     // Revolution circular buffer 
    uint8_t rev_sum;                            // Revolution summation for RPM calc 

    // User data 
    uint16_t rpm;                               // Calculated RPM 
    char rpm_buff[RPM_OUTPUT_BUFF_SIZE];        // Serial string to show RPM 
}
rpm_test_data_t; 

static rpm_test_data_t rpm_test_data; 

//=======================================================================================


//=======================================================================================
// Setup code 

// RPM test setup code 
void rpm_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Periodic (counter update) interrupt timer for RPM calculation. If the counter 
    // reload value changes, make sure to update the PRM_SAMPLE_PERIOD macro. 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0x1388,  // ARR=5000, (5000 counts)*(100us/count) = 500ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM10); 

    // Initialize UART - used to show the user the calculated RPM 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // External interrupt (rev count) setup 
    exti_init(); 
    exti_config(
        GPIOC, 
        EXTI_PC, 
        PIN_4, 
        PUPDR_PU, 
        EXTI_L4, 
        EXTI_INT_NOT_MASKED, 
        EXTI_EVENT_MASKED, 
        EXTI_RISE_TRIG_DISABLE, 
        EXTI_FALL_TRIG_ENABLE); 

    // Enable interrupts 
    nvic_config(EXTI4_IRQn, EXTI_PRIORITY_0);           // External - rev counter 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_1);   // Timer - PRM calc 

    // Initialize data 
    rpm_test_data.rev_count = CLEAR; 
    rpm_test_data.rev_buff_index = CLEAR; 
    memset((void *)rpm_test_data.rev_buff, CLEAR, sizeof(rpm_test_data.rev_buff)); 
    rpm_test_data.rev_sum = CLEAR; 
    rpm_test_data.rpm = CLEAR; 
    memset((void *)rpm_test_data.rpm_buff, CLEAR, sizeof(rpm_test_data.rpm_buff)); 
}

//=======================================================================================


//=======================================================================================
// Test code 

// RPM test application code 
void rpm_test_app(void)
{
    // The interrupt handler for the external interrupt is not used directly because the 
    // code loops quicker than there can be successive revolutions (for this test setup). 
    // The interrupt handler for the periodic interrpt is not used directly because a 
    // calculation needs to be done which is better suited to be handled here. 

    // External interrupt - revolution counter 
    if (handler_flags.exti4_flag)
    {
        handler_flags.exti4_flag = CLEAR; 
        rpm_test_data.rev_count++; 
    }

    // Periodic interrupt - RPM calculation 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Record the revolution count from the most recent invertal, update the circular 
        // buffer index and total the revolutions over the last RPM_SAMPLE_BUFF_SIZE 
        // intervals before calculating the RPM and outputting the result to the serial 
        // terminal for the user to see. 

        rpm_test_data.rev_buff[rpm_test_data.rev_buff_index++] = rpm_test_data.rev_count; 
        rpm_test_data.rev_count = CLEAR; 

        if (rpm_test_data.rev_buff_index >= RPM_SAMPLE_BUFF_SIZE)
        {
            rpm_test_data.rev_buff_index = CLEAR; 
        }

        rpm_test_data.rev_sum = CLEAR; 

        for (uint8_t i = CLEAR; i < RPM_SAMPLE_BUFF_SIZE; i++)
        {
            rpm_test_data.rev_sum += rpm_test_data.rev_buff[i]; 
        }

        rpm_test_data.rpm = (uint16_t)((double)rpm_test_data.rev_sum * RPM_SEC_TO_MIN / 
                                       (RPM_SAMPLE_BUFF_SIZE * PRM_SAMPLE_PERIOD)); 

        snprintf(
            rpm_test_data.rpm_buff, 
            RPM_OUTPUT_BUFF_SIZE, 
            "\rRPM: %u", 
            rpm_test_data.rpm); 
        uart_sendstring(USART2, rpm_test_data.rpm_buff); 
    }
}

//=======================================================================================
