/**
 * @file rc_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Remote control (RC) test 
 * 
 * @version 0.1
 * @date 2024-05-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "rc_test.h" 
#include "nrf24l01_config.h" 
#include "stm32f4xx_it.h" 

#include "nrf24l01_test.h" 
#include "hw125_test.h" 
#include "esc_readytosky_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

//==================================================
// Conditional compilation 

// Devices 
#define RC_SYSTEM_1 1 
#define RC_SYSTEM_2 0 

// Test code 
#define RC_SD_CARD_TEST 0 
#define RC_MOTOR_TEST 0 
#define RC_GROUND_STATION_TEST 1 

// Hardware 
#define RC_TEST_SCREEN 0        // HD44780U screen in the system - shuts screen off 

//==================================================

// Configuration 
#define NRF24L01_RF_FREQ 10           // Comm frequency: 2400 MHz + this value (MHz) 

//=======================================================================================


//=======================================================================================
// Classes 

// Device tracker data record 
class rc_test_trackers 
{
public: 
    // Timing information 
    TIM_TypeDef *timer_nonblocking;                // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                     // Delay timing info 

    // Configuration 
    nrf24l01_data_pipe_t pipe; 

    // Payload data 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN];   // Data read by PRX from PTX device 
    uint8_t write_buff[NRF24L01_MAX_PAYLOAD_LEN];  // Data sent to PRX from PTX device 
}; 

// Device tracker instance 
static rc_test_trackers rc_test; 

//=======================================================================================


//=======================================================================================
// Prototypes 

void rc_sd_card_test_init(void); 
void rc_sd_card_test_loop(void); 
void rc_motor_test_init(void); 
void rc_motor_test_loop(void); 
void rc_ground_station_test_init(void); 
void rc_ground_station_test_loop(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

// Remote control test setup code 
void rc_test_init(void)
{
    // Universal (to all nrf24l01 tests) setup code 

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Configure the board LED to blink when transmitting. 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(GPIOA, GPIOX_PIN_5, GPIO_LOW); 
    
    //==================================================

    //==================================================
    // UART initialization 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE);   // DMA enabled so it can be configured later 

    //==================================================

    //==================================================
    // Initialize I2C / screen 

#if RC_TEST_SCREEN 

    // Initialize the screen so it can be turned off 

    // Initialize I2C1 
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_backlight_off(); 

#endif   // RC_TEST_SCREEN 

    //==================================================

    //==================================================
    // Initialize SPI 

    // SPI for the RF module 
    spi_init(
        SPI2, 
        GPIOB,   // GPIO port for SCK pin 
        PIN_10,  // SCK pin 
        GPIOC,   // GPIO port for data (MISO/MOSI) pins 
        PIN_2,   // MISO pin 
        PIN_3,   // MOSI pin 
        SPI_BR_FPCLK_16, 
        SPI_CLOCK_MODE_0); 

    //==================================================

    //==================================================
    // Initialize test data 

    // Timing 
    rc_test.timer_nonblocking = TIM9; 
    rc_test.delay_timer.clk_freq = tim_get_pclk_freq(rc_test.timer_nonblocking); 
    rc_test.delay_timer.time_cnt_total = CLEAR; 
    rc_test.delay_timer.time_cnt = CLEAR; 
    rc_test.delay_timer.time_start = SET_BIT; 

    // Configuration 
    rc_test.pipe = NRF24L01_DP_1; 

    // Payload data 
    memset((void *)rc_test.read_buff, CLEAR, sizeof(rc_test.read_buff)); 
    memset((void *)rc_test.write_buff, CLEAR, sizeof(rc_test.write_buff)); 

    //==================================================

    //==================================================
    // Initialize the device driver 

    NRF24L01_STATUS nrf24l01_init_status = NRF24L01_OK; 

    // General setup common to all devices - must be called once during setup 
    nrf24l01_init_status |= nrf24l01_init(
        SPI2,                    // SPI port to use 
        GPIOC,                   // Slave select pin GPIO port 
        PIN_1,                   // Slave select pin number 
        GPIOC,                   // Enable pin (CE) GPIO port 
        PIN_0,                   // Enable pin (CE) number 
        TIM9,                    // General purpose timer port 
        NRF24L01_RF_FREQ,        // Initial RF channel frequency 
        NRF24L01_DR_2MBPS,       // Initial data rate to communicate at 
        NRF24L01_RF_PWR_0DBM);   // Initial power output to use 

    // Configure the PTX and PRX settings depending on the devices role/purpose. 
    nrf24l01_init_status |= nrf24l01_ptx_config(nrf24l01_pipe_addr); 
    nrf24l01_init_status |= nrf24l01_prx_config(nrf24l01_pipe_addr, rc_test.pipe); 

    // Power up the device now that it is configured 
    nrf24l01_init_status |= nrf24l01_pwr_up(); 

    // Check init status 
    if (nrf24l01_init_status)
    {
        uart_sendstring(USART2, "nRF24L01 init failed."); 
        while(1); 
    }
    else 
    {
        uart_sendstring(USART2, "nRF24L01 init success."); 
    }

    //==================================================

#if RC_SD_CARD_TEST 
    rc_sd_card_test_init(); 
#elif RC_MOTOR_TEST 
    rc_motor_test_init(); 
#elif RC_GROUND_STATION_TEST 
    rc_ground_station_test_init(); 
#endif 
}

//=======================================================================================


//=======================================================================================
// Test code 

// Remote control test code 
void rc_test_app(void)
{
#if RC_SD_CARD_TEST 
    rc_sd_card_test_loop(); 
#elif RC_MOTOR_TEST 
    rc_motor_test_loop(); 
#elif RC_GROUND_STATION_TEST 
    rc_ground_station_test_loop(); 
#endif 
}

//=======================================================================================


#if RC_SD_CARD_TEST 

//=======================================================================================
// SD card test 

// Description 
// - System 1: 
//   - Look for serial terminal input from a user. Valid inputs are "push" followed by 
//     a string or "pop". 
//   - If an inout matches a pre-defined command then execute the command callback. 
//   - Callback 1: ("push") 
//     - Send a push request to system 2. 
//     - If a response is seen then send the string/message to system 2. 
//     - If a confirmation is received then indicate success back to the user. 
//     - If no response is seen then it's considered a fail and system 1 goes back to 
//       looking for user input. 
//   - Callback 2: ("pop") 
//     - Send a pop request to system 2. 
//     - If a response is seen then output the response to the serial terminal for the 
//       user to see. 
// 
// - System 2: 
//   - Look for a message from system 1. 
//   - Attempt to match an input to a pre-defined command. If there is a match then 
//     the command callback is executed. 
//   - Callback 1: ("push") 
//     - Send a push confirmation message to system 1. 
//     - Wait for a message from system 1. 
//     - If a message is received then save it to the SD card and send a confirmation 
//       back to system 1. 
//   - Callback 2: ("pop") 
//     - Get the most recent message from the SD card file and send it back to system 
//       1. Delete the message from the file. 

//==================================================
// Macros 

// Timing 
#define RC_SD_PERIOD 50000          // (us) 
#define RC_SD_PUSH_MSG_TIMEOUT 20   // Counts 
#define RC_SD_PUSH_MSG_DELAY 10     // (ms) 

// Commands 
#define RC_SD_NUM_CMDS 2 

//==================================================


//==================================================
// Prototypes 

// "push" callback 
void rc_test_push_callback(
    uint8_t arg_value, 
    uint8_t *arg_str); 

// "pop" callback 
void rc_test_pop_callback(
    uint8_t arg_value, 
    uint8_t *arg_str); 

//==================================================


//==================================================
// Variables 

// Messages sent between system 
static const char 
push_cmd[] = "push",   // Doubles as a user command 
pop_cmd[] = "pop", 
push_confirm[] = "push confirm", 
msg_confirm[] = "msg confirm"; 

// Command table 
static const nrf24l01_cmds_t rc_cmd_table[RC_SD_NUM_CMDS] = 
{
    {push_cmd, &rc_test_push_callback}, 
    {pop_cmd,  &rc_test_pop_callback} 
};

#if RC_SYSTEM_1 

static const char 
push_status[] = "Push requested... ", 
push_success[] = "message saved.", 
pop_status[] = "Popped message: "; 

// Command data 
static nrf24l01_cmd_data_t rc_cmd_data; 

#elif RC_SYSTEM_2 

static const char 
no_msgs[] = "No messages to pop."; 

#endif 

//==================================================


//==================================================
// Setup 

void rc_sd_card_test_init(void)
{
#if RC_SYSTEM_1 

    //==================================================
    // UART DMA 

    // Enable the IDLE line interrupt 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // Initialize the DMA stream for the UART 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure the DMA stream for the UART 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)rc_cmd_data.cb, 
        (uint16_t)NRF24L01_MAX_PAYLOAD_LEN); 

    // Enable the DMA stream for the UART 
    dma_stream_enable(DMA1_Stream5); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    //==================================================

    memset((void *)rc_cmd_data.cb, CLEAR, sizeof(rc_cmd_data.cb)); 
    rc_cmd_data.cb_index = CLEAR; 
    memset((void *)rc_cmd_data.cmd_buff, CLEAR, sizeof(rc_cmd_data.cmd_buff)); 
    memset((void *)rc_cmd_data.cmd_id, CLEAR, sizeof(rc_cmd_data.cmd_id)); 
    rc_cmd_data.cmd_value = CLEAR; 

    // Provide an initial prompt for the user 
    uart_sendstring(USART2, "\r\n\n>>> "); 

#elif RC_SYSTEM_2 

    //==================================================
    // Initialize SPI 
    
    // The same SPI network (ex. SPI2) can't be used on multiple pins. Didn't work when 
    // initialization for the same SPI port on was done on multiple pins. Must use a 
    // different SPI port or only use one set of pins. 

    //==================================================

    //==================================================
    // Initialize SD card 

    // SD card user initialization 

    // Mount the drive 

    // Check for the test directory 
    // - If it doesn't exist then create it. 

    // Open the file with FA_OPEN_APPEND (HW125_MODE_AA) to either open the existing 
    // file or create a new one and move to the end of the file. 

    // Close the file. Will be opened for each read/write operation. 
    
    //==================================================
    
#endif 
}

//==================================================


//==================================================
// Loop 

void rc_sd_card_test_loop(void)
{
#if RC_SYSTEM_1 

    // Check for user input and match inputs to commands 
    nrf24l01_test_user_input(&rc_cmd_data, rc_cmd_table, RC_SD_NUM_CMDS, NRF24L01_CMD_ARG_STR); 

#elif RC_SYSTEM_2 

    // Periodically check for action items 
    if (tim_compare(rc_test.timer_nonblocking, 
                    rc_test.delay_timer.clk_freq, 
                    RC_SD_PERIOD, 
                    &rc_test.delay_timer.time_cnt_total, 
                    &rc_test.delay_timer.time_cnt, 
                    &rc_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Look for a message from system 1 
        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            nrf24l01_receive_payload(rc_test.read_buff); 
            
            // Compare the input to the available commands 
            for (uint8_t i = CLEAR; i < RC_SD_NUM_CMDS; i++) 
            {
                if (!strcmp(rc_cmd_table[i].user_cmds, (char *)rc_test.read_buff))
                {
                    // ID matched to a command. Execute the command callback. 
                    (rc_cmd_table[i].cmd_ptr)(CLEAR, NULL); 
                    break; 
                }
            }
        }
    }

#endif 
}

//==================================================


//==================================================
// Test functions 

// "push" callback 
void rc_test_push_callback(
    uint8_t arg_value, 
    uint8_t *arg_str)
{
#if RC_SYSTEM_1 

    // Send a "push" command to system 2 
    nrf24l01_send_payload((uint8_t *)push_cmd); 

    // Provide user feedback 
    uart_sendstring(USART2, push_status); 

    // Check repeatedly for a push confirmation message. If the confirmation is received 
    // within the time limit then send the message to system 2. 
    for (uint8_t i = CLEAR; i < RC_SD_PUSH_MSG_TIMEOUT; i++)
    {
        tim_delay_ms(rc_test.timer_nonblocking, RC_SD_PUSH_MSG_DELAY); 

        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            nrf24l01_receive_payload(rc_test.read_buff); 

            if (!strcmp(push_confirm, (char *)rc_test.read_buff))
            {
                // Send the message to system 2 
                // nrf24l01_send_payload(rc_cmd_data.cmd_str); 
                nrf24l01_send_payload(arg_str); 

                // Check repeatedly for a message received confirmation. If the 
                // confirmation is received within the time limit then provide feedback 
                // to the user that the operation was successful. 
                for (uint8_t j = CLEAR; j < RC_SD_PUSH_MSG_TIMEOUT; j++)
                {
                    tim_delay_ms(rc_test.timer_nonblocking, RC_SD_PUSH_MSG_DELAY); 

                    // Look for a message from system 2 
                    if (nrf24l01_data_ready_status() == rc_test.pipe)
                    {
                        nrf24l01_receive_payload(rc_test.read_buff); 

                        if (!strcmp(msg_confirm, (char *)rc_test.read_buff))
                        {
                            // Output a confirmation 
                            uart_sendstring(USART2, push_success); 
                            break; 
                        }
                    }
                }

                break; 
            }
        }
    }

#elif RC_SYSTEM_2 

    // Send a confirmation to system 1 that a "push" command was received 
    nrf24l01_send_payload((uint8_t *)push_confirm); 

    // Check multiple times (with a delay) if a message arrives 
    for (uint8_t i = CLEAR; i < RC_SD_PUSH_MSG_TIMEOUT; i++)
    {
        tim_delay_ms(rc_test.timer_nonblocking, RC_SD_PUSH_MSG_DELAY); 

        // Look for a message from system 1 
        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            nrf24l01_receive_payload(rc_test.read_buff); 

            // Save the contents to the end of the test file on the SD card 

            // Send another confirmation to system 1 that the message was received 
            nrf24l01_send_payload((uint8_t *)msg_confirm); 
        }
    }

#endif 
}


// "pop" callback 
void rc_test_pop_callback(
    uint8_t arg, 
    uint8_t *arg_str)
{
#if RC_SYSTEM_1 

    // Send a "pop" command to system 2 
    nrf24l01_send_payload((uint8_t *)pop_cmd); 

    // Provide user feedback 
    uart_sendstring(USART2, pop_status); 

    // Check repeatedly for a message from system 2. If a message is received then 
    // output it for the user to see. 
    for (uint8_t i = CLEAR; i < RC_SD_PUSH_MSG_TIMEOUT; i++)
    {
        tim_delay_ms(rc_test.timer_nonblocking, RC_SD_PUSH_MSG_DELAY); 

        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            nrf24l01_receive_payload(rc_test.read_buff); 
            uart_sendstring(USART2, (char *)rc_test.read_buff); 
            break; 
        }
    }

#elif RC_SYSTEM_2 

    // Get the most recent message from the test file on the SD card 
    // If there are none left then send the indication back to system 1. 

    // Send the message back to system 1 - temp message for now 
    nrf24l01_send_payload((uint8_t *)no_msgs); 

#endif 
}

//==================================================

//=======================================================================================


#elif RC_MOTOR_TEST 

//=======================================================================================
// Remote control motor test 

// Description 
// - System 1 reads two ADC values, one for each motor in system 2. The ADC values get 
//   converted to a throttle command and a message gets sent to system 2 that indicates 
//   the motor, direction and throttle to apply. 
// - System 2 looks for messages from system 1 that contain commands for the throttle to 
//   apply to each motor. The messages get decoded and the motor speed updated. If the 
//   system loses radio connection (i.e. no message/command is received after a period 
//   of time) then the motor speed is set to zero. 

//==================================================
// Macros 

// Command IDs 
#define RC_MOTOR_LEFT_MOTOR 0x4C          // "L" character that indicates left motor 
#define RC_MOTOR_RIGHT_MOTOR 0x52         // "R" character that indicates right motor 
#define RC_MOTOR_FWD_THRUST 0x50          // "P" (plus) - indicates forward thrust 
#define RC_MOTOR_REV_THRUST 0x4D          // "M" (minus) - indicates reverse thrust 
#define RC_MOTOR_NEUTRAL 0x4E             // "N" (neutral) - indicates zero thrust 
#define RC_MOTOR_NO_THRUST 0              // Force thruster output to zero 

// Timing 
#define RC_MOTOR_TIMEOUT 20               // No radio connection timeout counter 
#define RC_MOTOR_SEND_PERIOD 100000       // Time between throttle command sends (us) 
#define RC_MOTOR_RECEIVE_PERIOD 50000     // Time between throttle command sends (us) 

// System parameters 
#define RC_MOTOR_TEST_ADC_NUM 2           // Number of ADCs used for throttle command 
#define RC_MOTOR_ESC_PERIOD 20000         // ESC PWM timer period (auto-reload register) 
#define RC_MOTOR_ESC_FWD_SPEED_LIM 1600   // Forward PWM pulse time limit (us) 
#define RC_MOTOR_ESC_REV_SPEED_LIM 1440   // Reverse PWM pulse time limit (us) 

//==================================================


//==================================================
// Variables 

#if RC_SYSTEM_1 

// ADC storage - Location for the DMA to store ADC values 
static uint16_t adc_data[RC_MOTOR_TEST_ADC_NUM]; 

#elif RC_SYSTEM_2 

// Command data 
static nrf24l01_cmd_data_t rc_cmd_data; 

#endif 

//==================================================


//==================================================
// Prototypes 

#if RC_SYSTEM_1 
#elif RC_SYSTEM_2 

/**
 * @brief Thruster output 
 * 
 * @param timer : radio connection timeout counter 
 * @param device : device number (left vs right thruster) 
 * @param throttle : throttle command 
 */
void rc_test_thruster_output(
    uint8_t *timer, 
    device_number_t device, 
    int16_t throttle); 


/**
 * @brief Radio connection check 
 * 
 * @param timer : radio connection timeout counter 
 */
void rc_test_no_radio(uint8_t *timer); 

#endif 

//==================================================


//==================================================
// Setup 

void rc_motor_test_init(void)
{
#if RC_SYSTEM_1 

    //===================================================
    // ADC setup - for user controller mode 

    // Initialize the ADC port (called once) 
    adc1_clock_enable(RCC); 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE,      // ADC_EOC_EACH 
        ADC_PARAM_DISABLE,     // ADC_EOC_INT_DISABLE 
        ADC_PARAM_ENABLE,      // ADC_SCAN_ENABLE 
        ADC_PARAM_ENABLE,      // ADC_CONT_ENABLE 
        ADC_PARAM_ENABLE,      // ADC_DMA_ENABLE 
        ADC_PARAM_ENABLE,      // ADC_DDS_ENABLE 
        ADC_PARAM_DISABLE);    // ADC_OVR_INT_DISABLE 

    // Initialize the ADC pins and channels (called for each pin/channel) 
    // Right and left thruster control (potentiometer) in manual control mode 
    adc_pin_init(ADC1, GPIOA, PIN_6, ADC_CHANNEL_6, ADC_SMP_15); 
    adc_pin_init(ADC1, GPIOA, PIN_7, ADC_CHANNEL_7, ADC_SMP_15); 

    // Set the ADC conversion sequence (called for each sequence entry) 
    adc_seq(ADC1, ADC_CHANNEL_6, ADC_SEQ_1); 
    adc_seq(ADC1, ADC_CHANNEL_7, ADC_SEQ_2); 

    // Set the sequence length (called once and only for more than one channel) 
    adc_seq_len_set(ADC1, ADC_SEQ_2); 

    // Turn the ADC on 
    adc_on(ADC1); 
    
    //===================================================

    //==================================================
    // Initialize DMA 

    // Initialize the DMA stream for the ADC 
    dma_stream_init(
        DMA2, 
        DMA2_Stream0, 
        DMA_CHNL_0, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream for the ADC 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_data, 
        (uint16_t)RC_MOTOR_TEST_ADC_NUM); 

    // Enable the DMA stream for the ADC 
    dma_stream_enable(DMA2_Stream0); 

    // Start the ADC conversions (continuous mode) 
    adc_start(ADC1); 

    //==================================================

    memset((void *)adc_data, CLEAR, sizeof(adc_data)); 

#elif RC_SYSTEM_2 

    //==================================================
    // ESC/motor setup 

    // ESC driver setup 
    esc_readytosky_init(
        DEVICE_ONE, 
        TIM3, 
        TIMER_CH4, 
        GPIOB, 
        PIN_1, 
        TIM_84MHZ_1US_PSC, 
        RC_MOTOR_ESC_PERIOD, 
        RC_MOTOR_ESC_FWD_SPEED_LIM, 
        RC_MOTOR_ESC_REV_SPEED_LIM); 

    esc_readytosky_init(
        DEVICE_TWO, 
        TIM3, 
        TIMER_CH3, 
        GPIOB, 
        PIN_0, 
        TIM_84MHZ_1US_PSC, 
        RC_MOTOR_ESC_PERIOD, 
        RC_MOTOR_ESC_FWD_SPEED_LIM, 
        RC_MOTOR_ESC_REV_SPEED_LIM); 

    tim_enable(TIM3); 
    
    //==================================================

    memset((void *)rc_cmd_data.cb, CLEAR, sizeof(rc_cmd_data.cb)); 
    rc_cmd_data.cb_index = CLEAR; 
    memset((void *)rc_cmd_data.cmd_buff, CLEAR, sizeof(rc_cmd_data.cmd_buff)); 
    memset((void *)rc_cmd_data.cmd_id, CLEAR, sizeof(rc_cmd_data.cmd_id)); 
    rc_cmd_data.cmd_value = CLEAR; 

#endif 
}

//==================================================


//==================================================
// Loop 

void rc_motor_test_loop(void)
{
#if RC_SYSTEM_1 

    static gpio_pin_state_t led_state = GPIO_LOW; 
    static uint8_t thruster = CLEAR; 
    char side = CLEAR; 
    char sign = RC_MOTOR_FWD_THRUST; 
    int16_t throttle = CLEAR; 

    if (tim_compare(rc_test.timer_nonblocking, 
                    rc_test.delay_timer.clk_freq, 
                    RC_MOTOR_SEND_PERIOD, 
                    &rc_test.delay_timer.time_cnt_total, 
                    &rc_test.delay_timer.time_cnt, 
                    &rc_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Choose between right and left thruster 
        side = (thruster) ? RC_MOTOR_LEFT_MOTOR : RC_MOTOR_RIGHT_MOTOR; 

        // Read the ADC input and format the value for writing to the payload 
        throttle = esc_test_adc_mapping(adc_data[thruster]); 

        if (throttle == RC_MOTOR_NO_THRUST)
        {
            sign = RC_MOTOR_NEUTRAL; 
        }
        else if (throttle < RC_MOTOR_NO_THRUST)
        {
            // If the throttle is negative then change the value to positive and set the sign 
            // in the payload as negative. This helps on the receiving end. 
            throttle = ~throttle + 1; 
            sign = RC_MOTOR_REV_THRUST; 
        }

        // Format the payload with the thruster specifier and the throttle then send the 
        // payload. 
        snprintf(
            (char *)rc_test.write_buff, 
            NRF24L01_MAX_PAYLOAD_LEN, 
            "%c%c %d", 
            side, sign, throttle); 

        if (nrf24l01_send_payload(rc_test.write_buff) == NRF24L01_OK)
        {
            led_state = (gpio_pin_state_t)(GPIO_HIGH - led_state); 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 

        // Toggle the thruster flag 
        thruster = SET_BIT - thruster; 
    }

#elif RC_SYSTEM_2 

    static uint8_t timeout = CLEAR; 

    if (tim_compare(rc_test.timer_nonblocking, 
                    rc_test.delay_timer.clk_freq, 
                    RC_MOTOR_RECEIVE_PERIOD, 
                    &rc_test.delay_timer.time_cnt_total, 
                    &rc_test.delay_timer.time_cnt, 
                    &rc_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Check if a payload has been received 
        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            // Payload has been received. Read the payload from the device RX FIFO. 
            nrf24l01_receive_payload(rc_cmd_data.cmd_buff); 

            // Validate the payload format 
            if (nrf24l01_test_parse_cmd(&rc_cmd_data, NRF24L01_CMD_ARG_VALUE))
            {
                // Check that the command matches a valid throttle command. If it does then update 
                // the thruster command. Throttle command filtering is done by system 1. 

                int16_t cmd_value = (int16_t)rc_cmd_data.cmd_value; 
                int16_t throttle = RC_MOTOR_NO_THRUST; 
                uint8_t motor = rc_cmd_data.cmd_id[0]; 
                uint8_t direction = rc_cmd_data.cmd_id[1]; 

                // Determine the throttle command 
                if (direction == RC_MOTOR_FWD_THRUST)
                {
                    throttle = cmd_value; 
                }
                else if (direction == RC_MOTOR_REV_THRUST)
                {
                    throttle = ~cmd_value + 1; 
                }

                // Determine the motor 
                if (motor == RC_MOTOR_RIGHT_MOTOR)
                {
                    rc_test_thruster_output(&timeout, DEVICE_ONE, throttle); 
                }
                else if (motor == RC_MOTOR_LEFT_MOTOR)
                {
                    rc_test_thruster_output(&timeout, DEVICE_TWO, throttle); 
                }
            }
            else 
            {
                rc_test_no_radio(&timeout); 
            }
        }
        else 
        {
            rc_test_no_radio(&timeout); 
        }
    }

#endif 
}

//==================================================


//==================================================
// Test functions 

#if RC_SYSTEM_1 
#elif RC_SYSTEM_2 

// Thruster output 
void rc_test_thruster_output(
    uint8_t *timer, 
    device_number_t device, 
    int16_t throttle)
{
    // Radio connected - clear timeout 
    *timer = CLEAR; 
    esc_readytosky_send(device, throttle); 
}


// Radio connection check 
void rc_test_no_radio(uint8_t *timer)
{
    if (++*timer > RC_MOTOR_TIMEOUT)
    {
        rc_test_thruster_output(timer, DEVICE_ONE, RC_MOTOR_NO_THRUST); 
        rc_test_thruster_output(timer, DEVICE_TWO, RC_MOTOR_NO_THRUST); 
    }
}

#endif 

//==================================================

//=======================================================================================


#elif RC_GROUND_STATION_TEST 

//=======================================================================================
// Ground station test 

// Description 
// - Simple radio ground station that sends out inputs from the user and looks for 
//   responses. Inputs come from the serial terminal and responses are output to the 
//   serial terminal. This is not a two way test, but rather used to test any system 
//   that is controlled by a radio ground station. 

//==================================================
// Macros 

// Timing 
#define RC_GS_ACTION_PERIOD 100000     // Time between throttle command sends (us) 
#define RC_GS_HB_SEND_COUNTER 20       // This * RC_GS_ACTION_PERIOD gives HB send period 
#define RC_GS_HB_TIMEOUT_COUNTER 100   // This * RC_GS_ACTION_PERIOD gives HB timeout 

//==================================================


//==================================================
// Variables 

#if RC_SYSTEM_1 

// Command data 
static nrf24l01_cmd_data_t rc_gs_cmd_data; 
static const char 
ping_msg[] = "ping", 
ping_response[] = "pong", 
lost_connection[] = "Radio lost\033[1A"; 

#elif RC_SYSTEM_2 
#endif 

//==================================================


//==================================================
// Prototypes 

#if RC_SYSTEM_1 

/**
 * @brief User terminal prompt 
 */
void rc_ground_station_user_prompt(void); 

#elif RC_SYSTEM_2 
#endif 

//==================================================


//==================================================
// Setup 

void rc_ground_station_test_init(void)
{
#if RC_SYSTEM_1 

    //==================================================
    // UART DMA - further configure the UART for DMA input 

    // Enable the IDLE line interrupt 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // Initialize the DMA stream for the UART 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure the DMA stream for the UART 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)rc_gs_cmd_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)NRF24L01_TEST_MAX_INPUT); 

    // Enable the DMA stream for the UART 
    dma_stream_enable(DMA1_Stream5); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    //==================================================

    memset((void *)rc_gs_cmd_data.cb, CLEAR, sizeof(rc_gs_cmd_data.cb)); 
    rc_gs_cmd_data.cb_index = CLEAR; 
    memset((void *)rc_gs_cmd_data.cmd_buff, CLEAR, sizeof(rc_gs_cmd_data.cmd_buff)); 
    memset((void *)rc_gs_cmd_data.cmd_id, CLEAR, sizeof(rc_gs_cmd_data.cmd_id)); 
    rc_gs_cmd_data.cmd_value = CLEAR; 
    memset((void *)rc_gs_cmd_data.cmd_str, CLEAR, sizeof(rc_gs_cmd_data.cmd_str)); 

    rc_ground_station_user_prompt(); 

#elif RC_SYSTEM_2 
#endif 
}

//==================================================


//==================================================
// Loop 

void rc_ground_station_test_loop(void)
{
#if RC_SYSTEM_1 

    static uint8_t hb_send_counter = CLEAR; 
    static uint8_t hb_timeout_counter = CLEAR; 

    // Check for user serial terminal input 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(
            rc_gs_cmd_data.cb, 
            rc_gs_cmd_data.cmd_buff, 
            &rc_gs_cmd_data.cb_index, 
            NRF24L01_TEST_MAX_INPUT); 

        // Send string 
        nrf24l01_send_payload(rc_gs_cmd_data.cmd_buff); 

        rc_ground_station_user_prompt(); 
    }

    // Periodically check for action items 
    if (tim_compare(rc_test.timer_nonblocking, 
                    rc_test.delay_timer.clk_freq, 
                    RC_GS_ACTION_PERIOD, 
                    &rc_test.delay_timer.time_cnt_total, 
                    &rc_test.delay_timer.time_cnt, 
                    &rc_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Look for an incoming message from the remote system 
        if (nrf24l01_data_ready_status() == rc_test.pipe)
        {
            nrf24l01_receive_payload(rc_test.read_buff); 

            // Clear the timeout for any message received 
            hb_timeout_counter = CLEAR; 

            if (strcmp((char *)rc_test.read_buff, ping_response) != 0)
            {
                // Display the message for the ground station to see 
                uart_sendstring(USART2, "\033[1A\033[1A\r"); 
                uart_sendstring(USART2, (char *)rc_test.read_buff); 
                rc_ground_station_user_prompt(); 
            }
        }

        // Check if the radio connection had been lost for too long 
        if (hb_timeout_counter++ >= RC_GS_HB_TIMEOUT_COUNTER)
        {
            hb_timeout_counter = CLEAR; 
            
            // Display a lost connection message 
            uart_sendstring(USART2, "\033[1A\r"); 
            uart_sendstring(USART2, lost_connection); 
            rc_ground_station_user_prompt(); 
        }

        // Send a heartbeat message to the remote system periodically 
        if (hb_send_counter++ >= RC_GS_HB_SEND_COUNTER)
        {
            hb_send_counter = CLEAR; 
            nrf24l01_send_payload((uint8_t *)ping_msg); 
        }
    }

#elif RC_SYSTEM_2 
#endif 
}

//==================================================


//==================================================
// Test functions 

#if RC_SYSTEM_1 

// User terminal prompt 
void rc_ground_station_user_prompt(void)
{
    uart_sendstring(USART2, "\r\n\n>>> "); 
}

#elif RC_SYSTEM_2 
#endif 

//==================================================

//=======================================================================================

#endif 
