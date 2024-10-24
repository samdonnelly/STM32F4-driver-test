/**
 * @file wheel_rpm_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Wheel RPM (revolutions per minute) test 
 * 
 * @details This test determines the RPM of a wheel using a Hall Effect sensor and a 
 *          magnet. 
 * 
 * @version 0.1
 * @date 2024-10-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "wheel_rpm_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define RPM_SAMPLE_BUFF_SIZE 20   // Number of samples for RPM calculation 
#define RPM_OUTPUT_BUFF_SIZE 20   // Output string buffer size 
#define PRM_SAMPLE_PERIOD 200     // Time between samples (ms) 
#define RPM_SEC_TO_MIN 60         // 60 seconds / minute 

// Note: Time over which RPM is determined == PRM_SAMPLE_PERIOD * RPM_SAMPLE_BUFF_SIZE

//=======================================================================================


//=======================================================================================
// Global data 

typedef struct rpm_test_data_s 
{
    // Wheel revolution data 
    uint8_t rev_count;                          // Revolution counter 
    uint8_t rev_buff_index;                     // Revolution circular buffer index 
    uint8_t rev_buff[RPM_SAMPLE_BUFF_SIZE];     // Revolution circular buffer 
    uint32_t rev_sum;                           // Revolution summation for RPM calc 

    // User data 
    uint32_t rpm;                               // Calculated RPM 
    char rpm_buff[RPM_OUTPUT_BUFF_SIZE];        // Serial string to show RPM 
}
rpm_test_data_t; 

static rpm_test_data_t rpm_test_data; 

//=======================================================================================


//=======================================================================================
// Setup code 

// Wheel RPM test setup code 
void wheel_rpm_test_init(void)
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
        0x07D0,  // ARR=2000, (2000 counts)*(100us/count) = 200ms 
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

// Wheel RPM test application code 
void wheel_rpm_test_app(void)
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

        // RPM = (revolutions / (num_samples * sample_period[ms] / 1000[ms/s])) * 60[s/min] 
        rpm_test_data.rpm = (uint32_t)(rpm_test_data.rev_sum * RPM_SEC_TO_MIN * SCALE_1000 / 
                                      (RPM_SAMPLE_BUFF_SIZE * PRM_SAMPLE_PERIOD)); 
        
        // Note: Resolution for the RPM is dependent on the sample period (time between 
        //       revolution count checks) and sample buffer size (amount of past time to 
        //       look at when counting revolutions). These two values are multiplied 
        //       together and make up the denominator of the equation above. A higher 
        //       value of this multiplication gives a better/finer resolution whereas 
        //       a lower value gives a worse resolution. These two values work against 
        //       each other as more samples is better but increases time (i.e. the RPM 
        //       reading will lag in time) and shorter period is better but brings down 
        //       denominator value. You will have to pick a balance that works for you 
        //       but the ideal scenario is to have many samples and a short period while 
        //       still keeping the denominator large (i.e. sample number outweighs the 
        //       period length). 

        snprintf(
            rpm_test_data.rpm_buff, 
            RPM_OUTPUT_BUFF_SIZE, 
            "\rRPM: %lu  ", 
            rpm_test_data.rpm); 
        uart_sendstring(USART2, rpm_test_data.rpm_buff); 
    }
}

//=======================================================================================
