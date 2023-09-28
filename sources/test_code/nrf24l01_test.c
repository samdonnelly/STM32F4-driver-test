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


//=======================================================================================
// Function prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 

// Address sent by the PTX and address accepted by the PRX 
static uint8_t pipe_addr_buff[NRF24l01_ADDR_WIDTH] = {0xB3, 0xB4, 0xB5, 0xB6, 0x05}; 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;   // Timer used for non-blocking delays 
    tim_compare_t delay_timer;        // Delay timing info 

    // Data 
    uint8_t write_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test_trackers; 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 

#if NRF24L01_HEARTBEAT 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Data 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test_trackers; 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

//=======================================================================================


//=======================================================================================
// Setup code 

void nrf24l01_test_init(void)
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
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_TX_DISABLE, 
        UART_DMA_RX_DISABLE); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 
    
    //===================================================

    //===================================================
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

#if NRF24L01_MULTI_SPI && !NRF24L01_DEV1_CODE 

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

#endif   // NRF24L01_MULTI_SPI && NRF24L01_DEV1_CODE 

    //===================================================

    //===================================================
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
    nrf24l01_set_rf_pwr(NRF24L01_RF_PWR_0DBM); 

    // Configure the PTX and PRX settings depending on the devices role/purpose 
#if NRF24L01_DEV1_CODE 
    nrf24l01_ptx_config(pipe_addr_buff); 
#else   // NRF24L01_DEV1_CODE 
    nrf24l01_prx_config(pipe_addr_buff, NRF24L01_DP_1); 
#endif   // NRF24L01_DEV1_CODE 

    //===================================================

    //===================================================
    // Initialize SD card --> for testing multiple SPI pins on the same SPI bus 

#if NRF24L01_MULTI_SPI && !NRF24L01_DEV1_CODE 

    // SD card user initialization 
    hw125_user_init(SPI2, GPIOB, GPIOX_PIN_12); 

#endif   // NRF24L01_MULTI_SPI 
    
    //===================================================

    //==================================================
    // GPIO 

#if NRF24L01_DEV1_CODE 
    // Board LED - on when logic low 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
#else   // NRF24L01_DEV1_CODE 
#endif   // NRF24L01_DEV1_CODE 

    //==================================================

    //===================================================
    // Variable initialization 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

    // Timing information 
    nrf24l01_test_trackers.timer_nonblocking = TIM9; 
    nrf24l01_test_trackers.delay_timer.clk_freq = 
        tim_get_pclk_freq(nrf24l01_test_trackers.timer_nonblocking); 
    nrf24l01_test_trackers.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test_trackers.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test_trackers.delay_timer.time_start = SET_BIT; 

    // Data 
    memset((void *)nrf24l01_test_trackers.write_buff, CLEAR, 
           sizeof(nrf24l01_test_trackers.write_buff)); 
    strcpy((char *)nrf24l01_test_trackers.write_buff, "ping"); 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 
    
    // Data 
    memset((void *)nrf24l01_test_trackers.read_buff, CLEAR, 
           sizeof(nrf24l01_test_trackers.read_buff)); 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

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

void nrf24l01_test_app(void)
{
#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

    // Local variables 
    static gpio_pin_state_t led_state = GPIO_LOW; 

    // Periodically ping the second device 
    if (tim_compare(nrf24l01_test_trackers.timer_nonblocking, 
                    nrf24l01_test_trackers.delay_timer.clk_freq, 
                    NRF24L01_HB_PERIOD, 
                    &nrf24l01_test_trackers.delay_timer.time_cnt_total, 
                    &nrf24l01_test_trackers.delay_timer.time_cnt, 
                    &nrf24l01_test_trackers.delay_timer.time_start))
    {
        // nrf24l01_test_trackers.delay_timer.time_start = SET_BIT; 

        // Try sending out a payload and toggle the led if it was sent 
        if (nrf24l01_send_payload(nrf24l01_test_trackers.write_buff, 4))
        {
            led_state = GPIO_HIGH - led_state; 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 
    }

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 
    
    // Check if any data has been received 
    if (nrf24l01_data_ready_status(NRF24L01_DP_1))
    {
        // Data has been received. Read the payload from the device RX FIFO and display the 
        // data in the terminal.  
        nrf24l01_receive_payload(nrf24l01_test_trackers.read_buff, NRF24L01_DP_1); 
        nrf24l01_test_trackers.read_buff[4] = NULL_CHAR; 
        uart_sendstring(USART2, (char *)nrf24l01_test_trackers.read_buff); 
        uart_send_new_line(USART2); 
        memset((void *)nrf24l01_test_trackers.read_buff, CLEAR, 
               sizeof(nrf24l01_test_trackers.read_buff)); 
    }

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
