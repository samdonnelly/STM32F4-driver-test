/**
 * @file nrf24l01_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module test code 
 * 
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "nrf24l01_test.h" 

//=======================================================================================


// Heartbeat 
// - Device 1 (master) sends a 'ping' to device 2 (slave) periodically 
// - Device 2 reads ping and responds with a confirmation 
// - Device 1 reads confirmation to verify communication is working 
// - If device 1 doesn't get a response then indicate a loss of communication 
// - Option to enter a "configuration" mode (on both devices) where settings can be 
//   manually changed by the user. Can then switch back to heartbeat mode to make sure 
//   there is still communication. 

// Multi-SPI 
// - Device 1 (master) sends a user input message to device 2 (slave) upon user request 
// - Device 2 reads message and attempts to save it to a file on an SD card 
// - Device 2 reports back the status of the SD card write to device 1 

// RC 
// - Device 1 (master) sends throttle command to device 2 (slave) 
// - Device 2 reads throttle command 
// - Throttle command goes to the ESC driver 


//=======================================================================================
// Function prototypes 

/**
 * @brief Parse the user command into an ID and value 
 * 
 * @details 
 * 
 * @param command_id 
 * @param command_value 
 * @param command_buffer 
 * @return uint8_t 
 */
uint8_t nrf24l01_test_parse_cmd(
    uint8_t *command_buffer); 


/**
 * @brief RF channel set state 
 * 
 * @details 
 * 
 * @param rf_ch 
 */
void nrf24l01_test_rf_ch(
    uint8_t rf_ch); 


/**
 * @brief RF data rate set state 
 * 
 * @details 
 * 
 * @param rf_dr 
 */
void nrf24l01_test_rf_dr(
    uint8_t rf_dr); 


/**
 * @brief RF power output set state 
 * 
 * @details 
 * 
 * @param rf_pwr 
 */
void nrf24l01_test_rf_pwr(
    uint8_t rf_pwr); 


/**
 * @brief PRX device connection status 
 * 
 * @details 
 * 
 * @param status 
 */
void nrf24l01_test_status(
    uint8_t status); 


/**
 * @brief Invalid input user feedback 
 * 
 * @details 
 */
void nrf24l01_test_invalid_input(void); 

//=======================================================================================


//=======================================================================================
// Global variables 

//==================================================
// Data 

// Address sent by the PTX and address accepted by the PRX 
static uint8_t pipe_addr_buff[NRF24l01_ADDR_WIDTH] = {0xB3, 0xB4, 0xB5, 0xB6, 0x05}; 

// ADC storage 
static uint16_t adc_data[NRF24L01_TEST_ADC_NUM];  // Location for the DMA to store ADC values 

//==================================================

//==================================================
// Test data record 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;                // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                     // Delay timing info 

    // User commands 
    uint8_t user_buff[NRF24L01_TEST_MAX_INPUT];    // Circular buffer (CB) that stores user inputs 
    uint8_t buff_index;                            // CB index used for parsing commands 
    uint8_t cmd_buff[NRF24L01_TEST_MAX_INPUT];     // Stores a user command parsed from the CB 
    uint8_t cmd_id[NRF24L01_TEST_MAX_INPUT];       // Stores the ID of the user command 
    uint8_t cmd_value;                             // Stores the value of the user command 

    // Payload buffers 
    uint8_t hb_msg[NRF24L01_MAX_PAYLOAD_LEN];      // Heartbeat message 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN];   // Data read by PRX from PTX device 
    uint8_t write_buff[NRF24L01_MAX_PAYLOAD_LEN];  // Data sent to PRX from PTX device 

    // Status 
    uint8_t state;                                 // Test code "state" 
    uint8_t conn_status;                           // Device connection status 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test_data; 

//==================================================

//==================================================
// User commands 

// Command pointers 
typedef struct nrf24l01_user_cmds_s 
{
    char user_cmds[NRF24L01_TEST_MAX_INPUT]; 
    void (*nrf24l01_test_func_ptr)(uint8_t); 
    uint8_t cmd_mask; 
}
nrf24l01_user_cmds_t; 


// User commands 
static nrf24l01_user_cmds_t cmd_table[NRF24L01_NUM_USER_CMDS] = 
{
    {"rfch",   &nrf24l01_test_rf_ch,  0x3F}, 
    {"rfdr",   &nrf24l01_test_rf_dr,  0x3F}, 
    {"rfpwr",  &nrf24l01_test_rf_pwr, 0x3F}, 
    {"status", &nrf24l01_test_status, 0x38} 
}; 

//==================================================

//=======================================================================================


//=======================================================================================
// Setup code 

void nrf24l01_test_init(void)
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
        UART_DMA_ENABLE); 

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
    
    //==================================================

    //==================================================
    // Initialize DMA 

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
        (uint32_t)nrf24l01_test_data.user_buff, 
        (uint16_t)NRF24L01_TEST_MAX_INPUT); 

    // Enable the DMA stream for the UART 
    dma_stream_enable(DMA1_Stream5); 

    //==================================================

    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

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
    // Initialize the device driver 

    // General setup common to all device - must be called once during setup 
    nrf24l01_init(
        SPI2,    // SPI port to use 
        GPIOC,   // Slave select pin GPIO port 
        PIN_1,   // Slave select pin number 
        GPIOC,   // Enable pin (CE) GPIO port 
        PIN_0,   // Enable pin (CE) number 
        TIM9);   // General purpose timer port 

    // Set the devices initial communication parameters - can be updated as needed 
    nrf24l01_set_rf_channel(NRF24L01_RF_FREQ); 
    nrf24l01_set_rf_dr(NRF24L01_DR_2MBPS); 
    nrf24l01_set_rf_pwr(NRF24L01_RF_PWR_6DBM); 

    // PTX and PRX setup is required and is found below depending on the device 

    //==================================================

    //==================================================
    // Initialize variables 

    // Timing information 
    nrf24l01_test_data.timer_nonblocking = TIM9; 
    nrf24l01_test_data.delay_timer.clk_freq = 
        tim_get_pclk_freq(nrf24l01_test_data.timer_nonblocking); 
    nrf24l01_test_data.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test_data.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test_data.delay_timer.time_start = SET_BIT; 

    // User commands 
    memset((void *)nrf24l01_test_data.user_buff, CLEAR, sizeof(nrf24l01_test_data.user_buff)); 
    nrf24l01_test_data.buff_index = CLEAR; 
    memset((void *)nrf24l01_test_data.cmd_buff, CLEAR, sizeof(nrf24l01_test_data.cmd_buff)); 
    memset((void *)nrf24l01_test_data.cmd_id, CLEAR, sizeof(nrf24l01_test_data.cmd_id)); 
    nrf24l01_test_data.cmd_value = CLEAR; 

    // Payload data 
    memset((void *)nrf24l01_test_data.hb_msg, CLEAR, sizeof(nrf24l01_test_data.hb_msg)); 
    memset((void *)nrf24l01_test_data.read_buff, CLEAR, sizeof(nrf24l01_test_data.read_buff)); 
    memset((void *)nrf24l01_test_data.write_buff, CLEAR, sizeof(nrf24l01_test_data.write_buff)); 
    strcpy((char *)nrf24l01_test_data.hb_msg, "ping"); 

    // Status 
    nrf24l01_test_data.conn_status = CLEAR_BIT; 
    
    //==================================================


#if NRF24L01_DEV1_CODE   // Start of device 1 code 

    // Device one setup code 

    //==================================================
    // Initialize the device driver 

    // Configure the PTX settings depending on the devices role/purpose 
    nrf24l01_ptx_config(pipe_addr_buff); 

    //==================================================

    //==================================================
    // GPIO 

    // Board LED - on when logic low 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    //==================================================

#if NRF24L01_HEARTBEAT 
    
    // Device one - Heartbeat setup code 

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV1_HB_STATE; 
    
    //==================================================

#elif NRF24L01_MULTI_SPI 
    
    // Device one - Multiple SPI devices setup code 

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV1_MSPI_STATE; 
    
    //==================================================

#elif NRF24L01_RC 
    
    // Device one - RC setup code 

    //===================================================
    // ADC setup - for user controller mode 

    // Initialize the ADC port (called once) 
    adc1_clock_enable(RCC); 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE); 

    // Initialize the ADC pins and channels (called for each pin/channel) 
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
        DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Configure the DMA stream for the ADC 
    dma_stream_config(
        DMA2_Stream0, 
        (uint32_t)(&ADC1->DR), 
        (uint32_t)adc_data, 
        (uint16_t)NRF24L01_TEST_ADC_NUM); 

    // Enable the DMA stream for the ADC 
    dma_stream_enable(DMA2_Stream0); 

    // Start the ADC conversions (continuous mode) 
    adc_start(ADC1); 

    //==================================================

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV1_RC_STATE; 
    memset((void *)adc_data, CLEAR, sizeof(adc_data)); 
    
    //==================================================

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    
    // Device two setup code 

    //==================================================
    // Initialize the device driver 

    // Configure the PRX settings depending on the devices role/purpose 
    nrf24l01_prx_config(pipe_addr_buff, NRF24L01_DP_1); 

    //==================================================

#if NRF24L01_TEST_SCREEN 

    //==================================================
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

    //==================================================

#endif   // NRF24L01_TEST_SCREEN 

#if NRF24L01_HEARTBEAT 
    
    // Device two - Heartbeat setup code 

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV2_HB_STATE; 
    
    //==================================================

#elif NRF24L01_MULTI_SPI 
    
    // Device two - Multiple SPI devices setup code 

    //==================================================
    // Initialize SPI 

    // SPI2 and slave select pin for SD card 
    // This is on different pins than the RF module to test if the same SPI bus works across 
    // multiple pins. 
    spi_init(
        SPI2, 
        GPIOB,   // SCK pin GPIO port 
        PIN_10,  // SCK pin 
        GPIOB,   // Data (MISO/MOSI) pin GPIO port 
        PIN_14,  // MISO pin 
        PIN_15,  // MOSI pin 
        SPI_BR_FPCLK_16, 
        SPI_CLOCK_MODE_0); 
    spi_ss_init(GPIOB, PIN_12); 

    //==================================================

    //==================================================
    // Initialize SD card --> for testing multiple SPI pins on the same SPI bus 

    // SD card user initialization 
    hw125_user_init(SPI2, GPIOB, GPIOX_PIN_12); 
    
    //==================================================

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV2_MSPI_STATE; 
    
    //==================================================

#elif NRF24L01_RC 
    
    // Device two - RC setup code 

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

    // Enable the PWM timer 
    tim_enable(TIM3); 

    //=================================================== 

    //==================================================
    // Initialize variables 

    nrf24l01_test_data.state = NRF24L01_TEST_DEV2_RC_STATE; 
    
    //==================================================

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

    // Provide an initial user prompt 
    uart_sendstring(USART2, "\r\n>>> "); 
}

//=======================================================================================


//=======================================================================================
// Test code 

void nrf24l01_test_app(void)
{
    // Universal (to all nrf24l01 tests) application test code 

    // Check for user serial terminal input 
    if (handler_flags.usart2_flag)
    {
        // Reset the USART2 interrupt flag 
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(
            nrf24l01_test_data.user_buff, 
            nrf24l01_test_data.cmd_buff, 
            &nrf24l01_test_data.buff_index, 
            UART_TEST_MAX_INPUT); 

        // Validate the input - parse into an ID and value if valid 
        if (nrf24l01_test_parse_cmd(nrf24l01_test_data.cmd_buff))
        {
            // Valid input - compare the ID to each of the available pre-defined commands 
            for (uint8_t i = CLEAR; i < NRF24L01_NUM_USER_CMDS; i++) 
            {
                // Check that the command is available for the "state" before comparing it 
                // against the ID. 
                if (cmd_table[i].cmd_mask & (SET_BIT << nrf24l01_test_data.state))
                {
                    // Command available. Compare with the ID. 
                    if (str_compare(
                            cmd_table[i].user_cmds, 
                            (char *)nrf24l01_test_data.cmd_id, 
                            BYTE_0)) 
                    {
                        // ID matched to a command. Execute the command. 
                        (cmd_table[i].nrf24l01_test_func_ptr)(nrf24l01_test_data.cmd_value); 
                        break; 
                    }
                }
            }
        }

        uart_sendstring(USART2, "\r\n>>> "); 
    }

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

    // Device one application test code 

#if NRF24L01_HEARTBEAT 

    // Device one - Heartbeat test code 

    // Local variables 
    static gpio_pin_state_t led_state = GPIO_LOW; 

    // Periodically send a ping to the PRX device 
    if (tim_compare(nrf24l01_test_data.timer_nonblocking, 
                    nrf24l01_test_data.delay_timer.clk_freq, 
                    NRF24L01_HB_PERIOD, 
                    &nrf24l01_test_data.delay_timer.time_cnt_total, 
                    &nrf24l01_test_data.delay_timer.time_cnt, 
                    &nrf24l01_test_data.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Try sending out a payload and toggle the led if it was sent 
        if (nrf24l01_send_payload(nrf24l01_test_data.hb_msg))
        {
            led_state = GPIO_HIGH - led_state; 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 
    }

#elif NRF24L01_MULTI_SPI 
    
    // Device one - Multiple SPI test code 

#elif NRF24L01_RC 

    // Device one - RC test code 
    
    // Local variables 
    static gpio_pin_state_t led_state = GPIO_LOW; 
    static uint8_t thruster = CLEAR; 
    char side = CLEAR; 
    char sign = NRF24L01_FWD_THRUST; 
    int16_t throttle = CLEAR; 

    // Periodically send the throttle command - alternate between left and right side throttle 
    // commands for each send. 
    if (tim_compare(nrf24l01_test_data.timer_nonblocking, 
                    nrf24l01_test_data.delay_timer.clk_freq, 
                    NRF24L01_RC_PERIOD, 
                    &nrf24l01_test_data.delay_timer.time_cnt_total, 
                    &nrf24l01_test_data.delay_timer.time_cnt, 
                    &nrf24l01_test_data.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Choose between right and left thruster 
        side = (thruster) ? NRF24L01_LEFT_MOTOR : NRF24L01_RIGHT_MOTOR; 

        // Read the ADC input and format the value for writing to the payload 
        throttle = esc_test_adc_mapping(adc_data[thruster]); 
        if (throttle == NRF24L01_NO_THRUST)
        {
            sign = NRF24L01_NEUTRAL; 
        }
        else if (throttle < NRF24L01_NO_THRUST)
        {
            // If the throttle is negative then change the value to positive and set the sign 
            // in the payload as negative. This helps on the receiving end. 
            throttle = ~throttle + 1; 
            sign = NRF24L01_REV_THRUST; 
        }

        // Format the payload with the thruster specifier and the throttle then send the 
        // payload. 
        snprintf(
            (char *)nrf24l01_test_data.write_buff, 
            NRF24L01_MAX_PAYLOAD_LEN, 
            "%c%c%d", 
            side, sign, throttle); 

        if (nrf24l01_send_payload(nrf24l01_test_data.write_buff))
        {
            led_state = GPIO_HIGH - led_state; 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 

        // Toggle the thruster flag 
        thruster = SET_BIT - thruster; 
    }

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    
    // Device two application test code 

#if NRF24L01_HEARTBEAT 

    // Device two - Heartbeat test code 

    // Local variables 
    static uint8_t timeout_count = CLEAR; 

    // Increment the timeout counter periodically 
    if (tim_compare(nrf24l01_test_data.timer_nonblocking, 
                    nrf24l01_test_data.delay_timer.clk_freq, 
                    NRF24L01_HB_PERIOD, 
                    &nrf24l01_test_data.delay_timer.time_cnt_total, 
                    &nrf24l01_test_data.delay_timer.time_cnt, 
                    &nrf24l01_test_data.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Increment the timeout count until it's at the threshold at which point hold 
        // the count and clear the connection status. 
        if (timeout_count >= NRF24L01_HB_TIMEOUT)
        {
            nrf24l01_test_data.conn_status = CLEAR_BIT; 
        }
        else 
        {
            timeout_count++; 
        }
    }
    
    // Check if a payload has been received 
    if (nrf24l01_data_ready_status(NRF24L01_DP_1))
    {
        // Payload has been received. Read the payload from the device RX FIFO. 
        nrf24l01_receive_payload(nrf24l01_test_data.read_buff, NRF24L01_DP_1); 

        // Check to see if the received payload matches the heartbeat message 
        if (str_compare((char *)nrf24l01_test_data.hb_msg, 
                        (char *)nrf24l01_test_data.read_buff, 
                        BYTE_1))
        {
            // Heartbeat message received - reset the timeout and set the connection status 
            timeout_count = CLEAR; 
            nrf24l01_test_data.conn_status = SET_BIT; 
        }

        memset((void *)nrf24l01_test_data.read_buff, CLEAR, 
               sizeof(nrf24l01_test_data.read_buff)); 
    }

#elif NRF24L01_MULTI_SPI 
    
    // Device two - Multiple SPI test code 

#elif NRF24L01_RC 

    // Device two - RC test code 

    //==================================================
    // Note 
    // - Bit shifting works on signed integers - the sign bit is respected. 
    //==================================================
    
    // Local variables 
    static int16_t right_throttle = CLEAR; 
    static int16_t left_throttle = CLEAR; 
    int16_t cmd_value = CLEAR; 

    // Check if a payload has been received 
    if (nrf24l01_data_ready_status(NRF24L01_DP_1))
    {
        // Payload has been received. Read the payload from the device RX FIFO. 
        nrf24l01_receive_payload(nrf24l01_test_data.read_buff, NRF24L01_DP_1); 

        // Validate the payload format 
        if (nrf24l01_test_parse_cmd(&nrf24l01_test_data.read_buff[1]))
        {
            // Check that the command matches a valid throttle command. If it does then update 
            // the thruster command. 

            cmd_value = (int16_t)nrf24l01_test_data.cmd_value; 

            if (nrf24l01_test_data.cmd_id[0] == NRF24L01_RIGHT_MOTOR)
            {
                switch (nrf24l01_test_data.cmd_id[1])
                {
                    case NRF24L01_FWD_THRUST: 
                        right_throttle += (cmd_value - right_throttle) >> SHIFT_3; 
                        esc_readytosky_send(DEVICE_ONE, right_throttle); 
                        break; 
                    case NRF24L01_REV_THRUST: 
                        right_throttle += ((~cmd_value + 1) - right_throttle) >> SHIFT_3; 
                        esc_readytosky_send(DEVICE_ONE, right_throttle); 
                        break; 
                    case NRF24L01_NEUTRAL: 
                        if (cmd_value == NRF24L01_NO_THRUST)
                        {
                            right_throttle = NRF24L01_NO_THRUST; 
                            esc_readytosky_send(DEVICE_ONE, right_throttle); 
                        }
                        break; 
                    default: 
                        break; 
                }
            }
            else if (nrf24l01_test_data.cmd_id[0] == NRF24L01_LEFT_MOTOR)
            {
                switch (nrf24l01_test_data.cmd_id[1])
                {
                    case NRF24L01_FWD_THRUST: 
                        left_throttle += (cmd_value - left_throttle) >> SHIFT_3; 
                        esc_readytosky_send(DEVICE_TWO, left_throttle); 
                        break; 
                    case NRF24L01_REV_THRUST: 
                        left_throttle += ((~cmd_value + 1) - left_throttle) >> SHIFT_3; 
                        esc_readytosky_send(DEVICE_TWO, left_throttle); 
                        break; 
                    case NRF24L01_NEUTRAL: 
                        if (cmd_value == NRF24L01_NO_THRUST)
                        {
                            left_throttle = NRF24L01_NO_THRUST; 
                            esc_readytosky_send(DEVICE_TWO, left_throttle); 
                        }
                        break; 
                    default: 
                        break; 
                }
            }
        }

        memset((void *)nrf24l01_test_data.read_buff, CLEAR, 
               sizeof(nrf24l01_test_data.read_buff)); 
    }

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Parse the user command into an ID and value 
uint8_t nrf24l01_test_parse_cmd(
    uint8_t *command_buffer)
{
    // Local variables 
    uint8_t id_flag = SET_BIT; 
    uint8_t id_index = CLEAR; 
    uint8_t data = CLEAR; 
    uint8_t cmd_value[NRF24L01_TEST_MAX_INPUT]; 
    uint8_t value_size = CLEAR; 

    // Initialize data 
    memset((void *)nrf24l01_test_data.cmd_id, CLEAR, sizeof(nrf24l01_test_data.cmd_id)); 
    nrf24l01_test_data.cmd_value = CLEAR; 
    memset((void *)cmd_value, CLEAR, sizeof(cmd_value)); 

    // Parse the command into an ID and value 
    for (uint8_t i = CLEAR; command_buffer[i] != NULL_CHAR; i++)
    {
        data = command_buffer[i]; 

        if (id_flag)
        {
            // cmd ID parsing 

            id_index = i; 

            // Check that the command byte is within range 
            if ((data >= A_LO_CHAR && data <= Z_LO_CHAR) || 
                (data >= A_UP_CHAR && data <= Z_UP_CHAR))
            {
                // Valid character byte seen 
                nrf24l01_test_data.cmd_id[i] = data; 
            }
            else if (data >= ZERO_CHAR && data <= NINE_CHAR)
            {
                // Valid digit character byte seen 
                id_flag = CLEAR_BIT; 
                nrf24l01_test_data.cmd_id[i] = NULL_CHAR; 
                cmd_value[i-id_index] = data; 
                value_size++; 
            }
            else 
            {
                // Valid data not seen 
                return FALSE; 
            }
        }
        else 
        {
            // cmd value parsing 

            if (data >= ZERO_CHAR && data <= NINE_CHAR)
            {
                // Valid digit character byte seen 
                cmd_value[i-id_index] = data; 
                value_size++; 
            }
            else 
            {
                // Valid data not seen 
                return FALSE; 
            }
        }
    }

    // Calculate the cmd value 
    for (uint8_t i = CLEAR; i < value_size; i++)
    {
        nrf24l01_test_data.cmd_value += (uint8_t)char_to_int(cmd_value[i], value_size-i-1); 
    }

    return TRUE; 
}


// RF channel set state 
void nrf24l01_test_rf_ch(
    uint8_t rf_ch)
{
    // Check that input is within bounds 
    if (rf_ch <= NRF24L01_RF_CH_MAX)
    {
        nrf24l01_set_rf_channel(rf_ch); 
        if (nrf24l01_get_rf_ch() == rf_ch)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// RF data rate set state 
void nrf24l01_test_rf_dr(
    uint8_t rf_dr)
{
    // Check that input is within bounds 
    if (rf_dr <= (uint8_t)NRF24L01_DR_250KBPS)
    {
        nrf24l01_set_rf_dr((nrf24l01_data_rate_t)rf_dr); 
        if (nrf24l01_get_rf_dr() == (nrf24l01_data_rate_t)rf_dr)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// RF power output set state 
void nrf24l01_test_rf_pwr(
    uint8_t rf_pwr)
{
    // Check that input is within bounds 
    if (rf_pwr <= (uint8_t)NRF24L01_RF_PWR_0DBM)
    {
        nrf24l01_set_rf_pwr((nrf24l01_rf_pwr_t)rf_pwr); 
        if (nrf24l01_get_rf_pwr() == (nrf24l01_rf_pwr_t)rf_pwr)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// PRX device connection status 
void nrf24l01_test_status(
    uint8_t dummy_status)
{
    if (nrf24l01_test_data.conn_status)
    {
        uart_sendstring(USART2, "\r\nConnected.\r\n"); 
    }
    else 
    {
        uart_sendstring(USART2, "\r\nNot connected.\r\n"); 
    }
}


// Invalid input user feedback 
void nrf24l01_test_invalid_input(void)
{
    uart_sendstring(USART2, "\r\nInvalid cmd value.\r\n"); 
}

//=======================================================================================
