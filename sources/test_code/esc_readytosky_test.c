/**
 * @file esc_readytosky_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky ESC test code 
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

//=======================================================================================


//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Function prototypes 

#if ESC_CONTROLLER_MODE 

/**
 * @brief ADC to ESC command mapping 
 * 
 * @details Takes the raw ADC value and converts it to a throttle command between -100% 
 *          (full reverse) and 100% (full forward). Note that there is a deadband in the 
 *          middle of the ADC values (100-155) that will produce a 0% throttle command. 
 *          
 *          NOTE: This mapping is done under the assumption that the ADC is 8-bits - i.e. 
 *                reads from 0-255. 
 * 
 * @param adc_val : ADC input reading (0-255) 
 * @return int16_t : mapped throttle command to be sent to the ESC driver 
 */
int16_t esc_test_adc_mapping(
    uint16_t adc_val); 

#else   // ESC_CONTROLLER_MODE 

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

#if ESC_PARAM_ID 

#else   // ESC_PARAM_ID 

#if ESC_SECOND_DEVICE 

/**
 * @brief Select ESC 1 
 * 
 * @details 
 */
void esc_test_esc1_select(void); 


/**
 * @brief Select ESC 2 
 * 
 * @details 
 */
void esc_test_esc2_select(void); 

#endif   // ESC_SECOND_DEVICE 
#endif   // ESC_PARAM_ID 
#endif   // ESC_CONTROLLER_MODE 

//=======================================================================================


//=======================================================================================
// Global variables 

#if ESC_CONTROLLER_MODE 

// Data storage 
static uint16_t adc_data[ESC_ADC_BUFF_LEN];  // Location for the DMA to store ADC values 

#else   // ESC_CONTROLLER_MODE 

// User data 
static int16_t pwm_input; 
static char cmd_buff[ESC_INPUT_BUF_LEN]; 

#if ESC_PARAM_ID 

#else   // ESC_PARAM_ID 

// User data 
static device_number_t dev_num; 

#if ESC_SECOND_DEVICE 
    
// User data 
static uint8_t cmd_match_flag; 

// Command pointers 
typedef struct esc_test_user_cmds_s 
{
    char esc_cmds[ESC_INPUT_BUF_LEN];         // Stores the defined user input commands 
    void (*esc_test_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
esc_test_user_cmds_t; 

// User commands 
static esc_test_user_cmds_t cmd_table[ESC_NUM_TEST_CMDS] = 
{
    {"esc1", &esc_test_esc1_select}, 
    {"esc2", &esc_test_esc2_select} 
}; 

#endif   // ESC_SECOND_DEVICE 
#endif   // ESC_PARAM_ID 
#endif   // ESC_CONTROLLER_MODE 

//=======================================================================================


//=======================================================================================
// Setup code 

void esc_readytosky_test_init(void)
{
    //===================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 
    
    //===================================================

#if ESC_CONTROLLER_MODE 

    //===================================================
    // ESC driver setup 

    // ESC driver setup 
    esc_readytosky_init(
        DEVICE_ONE, 
        TIM3, 
        TIM_CHANNEL_4, 
        GPIOB, 
        PIN_1, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#if ESC_SECOND_DEVICE 

    esc_readytosky_init(
        DEVICE_TWO, 
        TIM3, 
        TIM_CHANNEL_3, 
        GPIOB, 
        PIN_0, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#endif   // ESC_SECOND_DEVICE 

    //=================================================== 

    //===================================================
    // ADC setup - for user controller mode 

    // Initialize the ADC port (called once) 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_EOC_EACH, 
#if ESC_SECOND_DEVICE   // For ADC scan mode 
        ADC_SCAN_ENABLE, 
#else 
        ADC_SCAN_DISABLE, 
#endif 
        ADC_CONT_ENABLE, 
        ADC_DMA_ENABLE, 
        ADC_DDS_ENABLE, 
        ADC_EOC_INT_DISABLE, 
        ADC_OVR_INT_DISABLE); 

    // Initialize the first ADC pin and channel (called for each pin/channel) 
    adc_pin_init(ADC1, GPIOA, PIN_6, ADC_CHANNEL_6, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_6, ADC_SEQ_1); 

#if ESC_SECOND_DEVICE 

    // Initialize the second ADC pin and channel 
    adc_pin_init(ADC1, GPIOA, PIN_7, ADC_CHANNEL_7, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_7, ADC_SEQ_2); 

    // Set the sequence length (called once and only for more than one channel) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

#endif   // ESC_SECOND_DEVICE 

    // Turn the ADC on 
    adc_on(ADC1); 
    
    //===================================================

    //===================================================
    // DMA setup 

    // Initialize the DMA stream 
    dma_stream_init(
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
#if ESC_SECOND_DEVICE   // Memeory increment 
        DMA_ADDR_INCREMENT, 
#else 
        DMA_ADDR_FIXED, 
#endif 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_data, 
        (uint16_t)ESC_ADC_BUFF_LEN); 

    // Enable the DMA stream 
    dma_stream_enable(DMA2_Stream0); 

    //===================================================

    // Start the ADC conversions 
    adc_start(ADC1); 

#else   // ESC_CONTROLLER_MODE 

#if ESC_PARAM_ID 

    //===================================================
    // Raw PWM command to the ESC 

    // PWM output timer 
    tim_2_to_5_output_init(
        TIM3, 
        TIM_CHANNEL_4, 
        GPIOB, 
        PIN_1, 
        TIM_DIR_UP, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 

    // Set the initial PWM value 
    tim_ccr(TIM3, ESC_NEUTRAL_TIME, TIM_CHANNEL_4); 

    tim_enable(TIM3); 
    
    //===================================================

#else   // ESC_PARAM_ID 

    //===================================================
    // ESC driver setup 

    // ESC driver setup 
    esc_readytosky_init(
        DEVICE_ONE, 
        TIM3, 
        TIM_CHANNEL_4, 
        GPIOB, 
        PIN_1, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#if ESC_SECOND_DEVICE 

    esc_readytosky_init(
        DEVICE_TWO, 
        TIM3, 
        TIM_CHANNEL_3, 
        GPIOB, 
        PIN_0, 
        TIM_84MHZ_1US_PSC, 
        ESC_PERIOD, 
        ESC_FWD_SPEED_LIM, 
        ESC_REV_SPEED_LIM); 

#endif   // ESC_SECOND_DEVICE 

    // Enable the PWM timer 
    tim_enable(TIM3); 

    //=================================================== 

#endif   // ESC_PARAM_ID 
#endif   // ESC_CONTROLLER_MODE 

    //===================================================
    // Variable init 

#if ESC_CONTROLLER_MODE 

    memset((void *)adc_data, CLEAR, sizeof(adc_data)); 

#else   // ESC_CONTROLLER_MODE 

    pwm_input = CLEAR; 
    memset((void *)cmd_buff, CLEAR, sizeof(cmd_buff)); 

#if ESC_PARAM_ID 
#else   // ESC_PARAM_ID 

    dev_num = DEVICE_ONE; 

#if ESC_SECOND_DEVICE 

    cmd_match_flag = CLEAR; 

#endif   // ESC_SECOND_DEVICE 
#endif   // ESC_PARAM_ID 
#endif   // ESC_CONTROLLER_MODE 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void esc_readytosky_test_app(void)
{
    // Local variables 

#if ESC_CONTROLLER_MODE 

    //===================================================
    // Manual joystick/knob control of the ESC(s) 

    // For testing the throttle command value: 
    uart_sendstring(USART2, "ESC 1 cmd: "); 
    uart_send_integer(USART2, esc_test_adc_mapping(adc_data[0])); 
    uart_send_new_line(USART2); 

    // // Convert the ADC value to a throttle command and send it to the ESC 
    // esc_readytosky_send(DEVICE_ONE, esc_test_adc_mapping(adc_data[0])); 

#if ESC_SECOND_DEVICE 

    // For testing the throttle command value: 
    uart_sendstring(USART2, "ESC 2 cmd: "); 
    uart_send_integer(USART2, esc_test_adc_mapping(adc_data[1])); 

    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 

    // esc_readytosky_send(DEVICE_TWO, esc_test_adc_mapping(adc_data[1])); 

#endif   // ESC_SECOND_DEVICE 

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 

    //===================================================

#else   // ESC_CONTROLLER_MODE 

    //===================================================
    // Get and format info from the user 

    uart_sendstring(USART2, "\r\n>>> "); 
    while(!uart_data_ready(USART2)); 

    // Retrieve and format the input 
    uart_getstr(USART2, cmd_buff, ESC_INPUT_BUF_LEN, UART_STR_TERM_CARRIAGE); 
    
    //===================================================

#if ESC_PARAM_ID 

    //===================================================
    // Raw PWM command to the ESC 

    // Check that the input is a valid number 
    if (esc_test_input_check(cmd_buff, &pwm_input))
    {
        // Compare the input to the allowable input range 
        if (pwm_input > ESC_FWD_SPEED_LIM)
        {
            pwm_input = ESC_FWD_SPEED_LIM; 
        }
        else if (pwm_input < ESC_REV_SPEED_LIM)
        {
            pwm_input = ESC_REV_SPEED_LIM; 
        }

        // Update the PWM value 
        tim_ccr(TIM3, (uint32_t)pwm_input, TIM_CHANNEL_4); 
    }
    else 
    {
        uart_sendstring(USART2, "\r\nInvalid input\r\n"); 
    }
    
    //===================================================
    
#else   // ESC_PARAM_ID 

    //===================================================
    // Serial terminal input control of ESC(s) 

    // Check that the input is a valid number 
    if (esc_test_input_check(cmd_buff, &pwm_input))
    {
        // Write PWM command to ESC/motor 
        esc_readytosky_send(dev_num, pwm_input); 
    }
    // If not a valid throttle command then check for user commands (if there are two devices) or 
    // otherwise invalidate the input 
    else 
    {

#if ESC_SECOND_DEVICE 

        for (uint8_t i = CLEAR; i < ESC_NUM_TEST_CMDS; i++)
        {
            if (str_compare(cmd_buff, cmd_table[i].esc_cmds, BYTE_0)) 
            {
                (cmd_table[i].esc_test_func_ptrs_t)(); 
                cmd_match_flag = SET_BIT; 
                break; 
            }
        }

        if (!cmd_match_flag)
        {
            uart_sendstring(USART2, "\r\nInvalid input\r\n"); 
        }

        cmd_match_flag = CLEAR;

#else   // ESC_SECOND_DEVICE 

        uart_sendstring(USART2, "\r\nInvalid input\r\n"); 

#endif   // ESC_SECOND_DEVICE 
    }

    //===================================================

#endif   // ESC_PARAM_ID 

#endif   // ESC_CONTROLLER_MODE 

    tim_delay_ms(TIM9, 50); 
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if ESC_CONTROLLER_MODE 

// ADC to ESC command mapping 
int16_t esc_test_adc_mapping(
    uint16_t adc_val)
{
    // Local variables 
    int16_t throttle_cmd = CLEAR;   // Assume 0% throttle and change if different 

    // Check if there is a forward or reverse throttle command 
    if (adc_val > ESC_ADC_FWD_LIM)
    {
        // Forward 
        throttle_cmd = adc_val - ESC_ADC_FWD_LIM; 
    }
    else if (adc_val < ESC_ADC_REV_LIM)
    {
        // Reverse 
        throttle_cmd = adc_val - ESC_ADC_REV_LIM; 
    }

    return throttle_cmd; 
}

#else   // ESC_CONTROLLER_MODE 

// User input check and conversion 
uint8_t esc_test_input_check(
    char *input_buff, 
    int16_t *input_num)
{
    // Local varaibles 
    uint8_t input_len = CLEAR; 
    int16_t digit = CLEAR; 
    char *buff_ptr = input_buff; 
    uint8_t sign = CLEAR; 

    // Clear the previous input number conversion 
    *input_num = CLEAR; 

    // Check for a minus sign first 
    if (*buff_ptr == MINUS_CHAR)
    {
        sign++; 
        buff_ptr++; 
        input_buff++; 
    }

    // Check that all the characters are digits and get the input length 
    for (uint8_t i = 0; i < (ESC_INPUT_MAX_LEN - sign); i++)
    {
        if (*buff_ptr == CR_CHAR)
        {
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

    // Convert the input to a number 
    for (uint8_t i = CLEAR; i < input_len; i++)
    {
        digit = (int16_t)(*input_buff++ - NUM_TO_CHAR_OFFSET); 
        *input_num += digit*(int16_t)pow((double)10, (double)(input_len-i-1)); 
    }

    // If the input is negative then add a negative sign 
    if (sign)
    {
        *input_num = ~(*input_num) + 1; 
    }

    return TRUE; 
}

#if ESC_PARAM_ID 
#else   // ESC_PARAM_ID 
#if ESC_SECOND_DEVICE 

// Select ESC 1 
void esc_test_esc1_select(void)
{
    dev_num = DEVICE_ONE; 
}


// Select ESC 2 
void esc_test_esc2_select(void)
{
    dev_num = DEVICE_TWO; 
}

#endif   // ESC_SECOND_DEVICE 
#endif   // ESC_PARAM_ID 
#endif   // ESC_CONTROLLER_MODE 

//=======================================================================================
