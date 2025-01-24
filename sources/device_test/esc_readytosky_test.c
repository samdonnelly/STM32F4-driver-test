/**
 * @file esc_readytosky_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky ESC test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * One or two ESCs connected to the STM32F4. 
 *            * A motor connected to each ESC. 
 *            * A potentiometer for each ESC/motor connected to the STM32F4. 
 *          
 *          Configuration 
 *          - UART 
 *            * One UART port configured for a serial terminal connection. 
 *          - ADC 
 *            * ADC inputs configured for each potentiometer to read their input 
 *              continuously. 
 *          - DMA 
 *            * Two DMA streams configured, one for serial terminal input from the user 
 *              and another for ADC readings from the poteniometers. Serial terminal is 
 *              transferred when there is RX data, ADC is continuous. 
 *          - Interrupts 
 *            * Two interrupts configured, one for serial terminal input when the RX 
 *              line goes idle and another is a periodic interrupt to update the ESC 
 *              PWM output at a fixed interval when controlling the ESC with ADC. 
 *          - ESCs 
 *            * One or two ESCs configured using the ESC driver which sets up a timer 
 *              to output a specified PWM to the ESC. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - UART, ADC, DMA, interrupts and ESC drivers are used. 
 *          
 *          Procedure 
 *          - The test has two modes: command and control mode. Command mode controls 
 *            output to the ESCs via commands input from a serial terminal. The user can 
 *            input a valid throttle value and the code will translate that into a PWM 
 *            signal to send to the ESCs. Control mode determines the PWM value sent to 
 *            the ESCs by reading the value of ADC inputs controlled by potentiometers. 
 *          - The code will idle until at least one of two interrupts are triggered. The 
 *            first interrupt is a periodic timer interrupt which will trigger at a fixed 
 *            interval and convert the current ADC reading to a PWM signal that gets 
 *            send to the ESCs if in control mode. The second interrupt is triggered when 
 *            new data is received via UART from the serial terminal. Once data is 
 *            received, the input will be processed. The input can either be a number 
 *            representing a throttle value or it can be a pre-defined command. A 
 *            throttle input will only have an affect when in command mode. All other 
 *            inputs in any mode will be checked to see if they match existing commands 
 *            and if a match is found then the appropriate action will be taken. Upon 
 *            successful command match a confirmation will be sent to the serial terminal 
 *            so the user knows the command worked. 
 * 
 * @version 0.1
 * @date 2023-09-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "esc_readytosky_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Parameters 
#define ESC_TEST_PERIOD 20000            // ESC PWM timer period (auto-reload register) 
#define ESC_TEST_FWD_SPEED_LIM 1600      // Forward PWM pulse time limit (us) 
#define ESC_TEST_REV_SPEED_LIM 1440      // Reverse PWM pulse time limit (us) 

// User input 
#define ESC_TEST_MAX_INPUT 30            // Max user input size (bytes) 
#define ESC_TEST_NUM_TEST_CMDS 4         // Number of user commands available 

// Data 
#define ESC_TEST_NUM_ADC 2               // Number of ADCs used 
#define ESC_TEST_ADC_REV_LIM 100         // ADC value reverse command limit 
#define ESC_TEST_ADC_FWD_LIM 155         // ADC value forward command limit 

//=======================================================================================


//=======================================================================================
// Enums 

typedef enum {
    ESC_TEST_MODE_COMMAND, 
    ESC_TEST_MODE_CONTROL 
} esc_test_mode_t; 

//=======================================================================================


//=======================================================================================
// Global variables 

typedef struct esc_test_data_s
{
    esc_test_mode_t mode_flag; 

    // Remote control mode 
    uint16_t adc_data[ESC_TEST_NUM_ADC]; 

    // Command mode 
    int16_t pwm_input; 
    device_number_t dev_num; 

    // User inputs 
    USART_TypeDef *uart; 
    DMA_Stream_TypeDef *dma_stream; 
    uint8_t cb[ESC_TEST_MAX_INPUT];          // Circular buffer populated by DMA 
    cb_index_t cb_index;                     // Circular buffer indexing info 
    dma_index_t dma_index;                   // DMA transfer indexing info 
    uint8_t data_buff[ESC_TEST_MAX_INPUT];   // Buffer that stores latest UART input 
}
esc_test_data_t; 

static esc_test_data_t esc_data; 


// Command pointers 
typedef struct esc_test_cmds_s 
{
    char esc_cmds[ESC_TEST_MAX_INPUT];       // Stores the defined user input commands 
    void (*esc_test_func_ptrs_t)(void);      // Pointer to FatFs file operation function 
}
esc_test_cmds_t; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief User input check and conversion 
 * 
 * @param input_buff : stores to raw user input 
 * @param input_num : stores the converted number input 
 * @return uint8_t : status of the input check --> TRUE : valid, FALSE : invalid 
 */
uint8_t esc_test_input_check(
    char *input_buff, 
    int16_t *input_num); 


// UI functions 
void esc_test_user_prompt(void);     // User prompt 
void esc_test_user_feedback(void);   // User feedback 

// Command callbacks 
void esc_test_cmd_esc1_select(void);    // Select ESC 1 
void esc_test_cmd_esc2_select(void);    // Select ESC 2 
void esc_test_cmd_command_mode(void);   // Select command mode 
void esc_test_cmd_control_mode(void);   // Select control mode 

//=======================================================================================


//=======================================================================================
// Command table 

static esc_test_cmds_t esc_cmd_table[ESC_TEST_NUM_TEST_CMDS] = 
{
    {"esc1", &esc_test_cmd_esc1_select}, 
    {"esc2", &esc_test_cmd_esc2_select}, 
    {"command", &esc_test_cmd_command_mode}, 
    {"control", &esc_test_cmd_control_mode} 
}; 

//=======================================================================================


//=======================================================================================
// Setup code 

void esc_readytosky_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x03E8,   // ARR=1000, (1000 counts)*(100us/count) = 100ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    //==================================================
    // UART 

    // UART2 init - Serial terminal 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 
    
    //==================================================

    //==================================================
    // ADC 

    adc1_clock_enable(RCC); 

    // Initialize the ADC port 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
#if ESC_SECOND_DEVICE   // ADC scan mode 
        ADC_PARAM_ENABLE, 
#else 
        ADC_PARAM_DISABLE, 
#endif 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize the first ADC pin and channel, as well as the conversion sequence and 
    // length. 
    adc_pin_init(ADC1, GPIOA, PIN_6, ADC_CHANNEL_6, ADC_SMP_15); 
    adc_seq(ADC1, ADC_CHANNEL_6, ADC_SEQ_1); 

#if ESC_SECOND_DEVICE 

    // Initialize the second ADC pin and channel, as well as the conversion sequence and 
    // length. 
    adc_pin_init(ADC1, GPIOA, PIN_7, ADC_CHANNEL_7, ADC_SMP_15); 
    adc_seq(ADC1, ADC_CHANNEL_7, ADC_SEQ_2); 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

#endif   // ESC_SECOND_DEVICE 

    adc_on(ADC1); 

    //==================================================

    //==================================================
    // DMA 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_HI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA1 stream config - UART2 - Serial terminal 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)esc_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)ESC_TEST_MAX_INPUT); 

    // DMA2 stream init - ADC1 - control knobs 
    dma_stream_init(
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_DBM_DISABLE, 
#if ESC_SECOND_DEVICE         // Memeory increment 
        DMA_ADDR_INCREMENT, 
#else 
        DMA_ADDR_FIXED, 
#endif 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // DMA2 stream config - ADC1 - control knobs 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)esc_data.adc_data, 
        (uint32_t)NULL, 
        (uint16_t)ESC_TEST_NUM_ADC); 

    // Enable DMA streams 
    dma_stream_enable(DMA1_Stream5);   // UART2 - Serial terminal 
    dma_stream_enable(DMA2_Stream0);   // ADC1 - control knobs 

    //==================================================

    //==================================================
    // Interrupts 

    // Initialize interrupt handler flags and enable the handlers 
    int_handler_init(); 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0);          // UART2 - Serial terminal 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_1);   // TIM9 - Periodic ESC update 

    //==================================================

    //==================================================
    // ESC config 

    // ESC driver setup 
    esc_readytosky_init(
        DEVICE_ONE, 
        TIM3, 
        TIMER_CH4, 
        GPIOB, 
        PIN_1, 
        TIM_84MHZ_1US_PSC, 
        ESC_TEST_PERIOD, 
        ESC_TEST_FWD_SPEED_LIM, 
        ESC_TEST_REV_SPEED_LIM); 

#if ESC_SECOND_DEVICE 

    esc_readytosky_init(
        DEVICE_TWO, 
        TIM3, 
        TIMER_CH3, 
        GPIOB, 
        PIN_0, 
        TIM_84MHZ_1US_PSC, 
        ESC_TEST_PERIOD, 
        ESC_TEST_FWD_SPEED_LIM, 
        ESC_TEST_REV_SPEED_LIM); 

#endif   // ESC_SECOND_DEVICE 

    // Enable the PWM timer 
    tim_enable(TIM3); 

    //==================================================

    // Start the ADC conversions 
    adc_start(ADC1); 

    // Initialize data 
    esc_data.mode_flag = ESC_TEST_MODE_COMMAND; 
    memset((void *)esc_data.adc_data, CLEAR, sizeof(esc_data.adc_data)); 
    esc_data.pwm_input = CLEAR; 
    esc_data.dev_num = DEVICE_ONE; 
    esc_data.uart = USART2; 
    esc_data.dma_stream = DMA1_Stream5; 
    memset((void *)esc_data.cb, CLEAR, sizeof(esc_data.cb)); 
    esc_data.cb_index.cb_size = ESC_TEST_MAX_INPUT; 
    esc_data.cb_index.head = CLEAR; 
    esc_data.cb_index.tail = CLEAR; 
    esc_data.dma_index.data_size = CLEAR; 
    esc_data.dma_index.ndt_old = dma_ndt_read(esc_data.dma_stream); 
    esc_data.dma_index.ndt_new = CLEAR; 
    memset((void *)esc_data.data_buff, CLEAR, sizeof(esc_data.data_buff)); 

    esc_test_user_prompt(); 
}

//=======================================================================================


//=======================================================================================
// Test code 

void esc_readytosky_test_app(void)
{
    // User input interrupt 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        dma_cb_index(esc_data.dma_stream, &esc_data.dma_index, &esc_data.cb_index); 
        cb_parse(esc_data.cb, &esc_data.cb_index, esc_data.data_buff); 

        // If in command mode and the input is a valid number then update the PWM 
        // command to send to the selected ESC. 
        if ((esc_data.mode_flag == ESC_TEST_MODE_COMMAND) && 
             esc_test_input_check(esc_data.data_buff, &esc_data.pwm_input))
        {
            // Write PWM command to ESC/motor 
            esc_readytosky_send(esc_data.dev_num, esc_data.pwm_input); 
            esc_test_user_feedback(); 
        }
        else 
        {
            // If the input is not a number then it may be a command. Check if the input 
            // matches any pre-defined commands. If not then the input is invalid. 
            for (uint8_t i = CLEAR; i < ESC_TEST_NUM_TEST_CMDS; i++)
            {
                if (str_compare(esc_cmd_table[i].esc_cmds, esc_data.data_buff, BYTE_0)) 
                {
                    (esc_cmd_table[i].esc_test_func_ptrs_t)(); 
                    esc_test_user_feedback(); 
                    break; 
                }
            }
        }

        esc_test_user_prompt(); 
    }

    // Periodic interrupt to update ESC command 
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR; 

        if (esc_data.mode_flag == ESC_TEST_MODE_CONTROL)
        {
            // Convert the ADC value to a throttle command and send it to the ESC 
            esc_readytosky_send(DEVICE_ONE, esc_test_adc_mapping(esc_data.adc_data[0])); 
#if ESC_SECOND_DEVICE 
            esc_readytosky_send(DEVICE_TWO, esc_test_adc_mapping(esc_data.adc_data[1])); 
#endif   // ESC_SECOND_DEVICE 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// User input check and conversion 
uint8_t esc_test_input_check(
    char *input_buff, 
    int16_t *input_num)
{
    if ((input_buff == NULL) || (input_num == NULL))
    {
        return FALSE; 
    }

    char *buff_ptr = input_buff; 
    uint8_t input_len = CLEAR; 
    int16_t digit = CLEAR; 
    uint8_t sign = CLEAR; 

    // Check for a minus sign first 
    if (*buff_ptr == MINUS_CHAR)
    {
        buff_ptr++; 
        input_buff++; 
        sign++; 
    }

    // Check that all the characters are digits and get the input length 
    // for (uint8_t i = CLEAR; i < (ESC_TEST_MAX_INPUT - sign); i++)
    while (buff_ptr != NULL)
    {
        if ((*buff_ptr == CR_CHAR) || (*buff_ptr == NL_CHAR) || (*buff_ptr == NULL_CHAR))
        {
            if (input_len == ZERO)
            {
                return FALSE; 
            }

            break; 
        }

        // If the character is not a digit then it's an invalid input 
        if ((*buff_ptr < ZERO_CHAR) || (*buff_ptr > NINE_CHAR))
        {
            return FALSE; 
        }

        input_len++; 
        buff_ptr++; 
    }

    *input_num = CLEAR; 

    // Convert the input to a number 
    for (uint8_t i = CLEAR; i < input_len; i++)
    {
        digit = (int16_t)(*input_buff++ - NUM_TO_CHAR_OFFSET); 
        *input_num += digit*(int16_t)pow((double)10, (double)(input_len-i-1)); 
    }

    // If the input is negative then add a negative sign 
    if (sign)
    {
        *input_num = -(*input_num); 
    }

    return TRUE; 
}


// ADC to ESC command mapping 
int16_t esc_test_adc_mapping(uint16_t adc_val)
{
    int16_t throttle_cmd = CLEAR;   // Assume 0% throttle and change if different 

    // Check if there is a forward or reverse throttle command 
    if (adc_val > ESC_TEST_ADC_FWD_LIM)
    {
        // Forward 
        throttle_cmd = (int16_t)adc_val - ESC_TEST_ADC_FWD_LIM; 
    }
    else if (adc_val < ESC_TEST_ADC_REV_LIM)
    {
        // Reverse 
        throttle_cmd = (int16_t)adc_val - ESC_TEST_ADC_REV_LIM; 
    }

    return throttle_cmd; 
}


// User prompt 
void esc_test_user_prompt(void)
{
    uart_send_str(esc_data.uart, "\r\n>>> "); 
}


// User feedback 
void esc_test_user_feedback(void)
{
    uart_send_str(esc_data.uart, "\r\nconfirmed\r\n"); 
}


// Select ESC 1 
void esc_test_cmd_esc1_select(void)
{
    esc_data.dev_num = DEVICE_ONE; 
}


// Select ESC 2 
void esc_test_cmd_esc2_select(void)
{
    esc_data.dev_num = DEVICE_TWO; 
}


// Select command mode 
void esc_test_cmd_command_mode(void)
{
    esc_data.mode_flag = ESC_TEST_MODE_COMMAND; 
}


// Select control mode 
void esc_test_cmd_control_mode(void)
{
    esc_data.mode_flag = ESC_TEST_MODE_CONTROL; 
}

//=======================================================================================
