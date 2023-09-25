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

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;             // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                  // Delay timing info 

    // State 
    uint8_t state; 
    uint8_t ping; 

    // Data 
    uint8_t read_buff[NRF24L01_MAX_PACK_LEN]; 
    uint8_t write_buff[NRF24L01_MAX_PACK_LEN]; 
    uint8_t response[NRF24L01_MAX_PACK_LEN]; 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t trackers; 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 

#if NRF24L01_HEARTBEAT 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Data 
    uint8_t read_buff[NRF24L01_MAX_PACK_LEN]; 
    uint8_t write_buff[NRF24L01_MAX_PACK_LEN]; 
    uint8_t message[NRF24L01_MAX_PACK_LEN]; 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t trackers; 

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

    nrf24l01_init(
        SPI2, 
        GPIOC,   // Slave select pin GPIO port 
        PIN_1,   // Slave select pin number 
        GPIOC,   // Enable pin (CE) GPIO port 
        PIN_0,   // Enable pin (CE) number 
        TIM9, 
        NRF24L01_DR_250KBPS, 
        NRF24L01_RF_FREQ); 

    //===================================================

    //===================================================
    // Initialize SD card --> for testing multiple SPI pins on the same SPI bus 

#if NRF24L01_MULTI_SPI && !NRF24L01_DEV1_CODE 

    // SD card user initialization 
    hw125_user_init(SPI2, GPIOB, GPIOX_PIN_12); 

#endif   // NRF24L01_MULTI_SPI 
    
    //===================================================

    //===================================================
    // Variable initialization 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

    // Timing information 
    trackers.timer_nonblocking = TIM9; 
    trackers.delay_timer.clk_freq = tim_get_pclk_freq(trackers.timer_nonblocking); 
    trackers.delay_timer.time_cnt_total = CLEAR; 
    trackers.delay_timer.time_cnt = CLEAR; 
    trackers.delay_timer.time_start = SET_BIT; 

    // State 
    trackers.state = CLEAR_BIT; 
    trackers.ping = CLEAR_BIT; 

    // Data 
    memset((void *)trackers.read_buff, CLEAR, sizeof(trackers.read_buff)); 
    memset((void *)trackers.write_buff, CLEAR, sizeof(trackers.write_buff)); 
    memset((void *)trackers.response, CLEAR, sizeof(trackers.response)); 
    strcpy((char *)trackers.write_buff, "ping"); 
    strcpy((char *)trackers.response, "ping_confirm"); 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 
    
    // Data 
    memset((void *)trackers.read_buff, CLEAR, sizeof(trackers.read_buff)); 
    memset((void *)trackers.write_buff, CLEAR, sizeof(trackers.write_buff)); 
    memset((void *)trackers.message, CLEAR, sizeof(trackers.message)); 
    strcpy((char *)trackers.write_buff, "ping_confirm"); 
    strcpy((char *)trackers.message, "ping"); 

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

    // Periodically ping the second device 
    if (tim_compare(trackers.timer_nonblocking, 
                    trackers.delay_timer.clk_freq, 
                    NRF24L01_HB_PERIOD, 
                    &trackers.delay_timer.time_cnt_total, 
                    &trackers.delay_timer.time_cnt, 
                    &trackers.delay_timer.time_start))
    {
        // trackers.delay_timer.time_start = SET_BIT; 

        // Check to see if a ping was received 
        if (trackers.ping)
        {
            // Ping seen. Clear the ping flag so we have to search for a new ping and check if 
            // we are currently in the not seen state. 
            trackers.ping = CLEAR_BIT; 
            if (!trackers.state)
            {
                // Ping not seen state. Change the state flag and send an update message that a 
                // ping has now been seen 
                trackers.state = SET_BIT; 
                uart_sendstring(USART2, "\r\nDevice found.\r\n"); 
            }
        }
        else
        {
            // Ping not seen. Check if we are currently in the seen state. 
            if (trackers.state)
            {
                // Ping seen state. Change the state flag and send an update message that the 
                // device has been lost. 
                trackers.state = CLEAR_BIT; 
                uart_sendstring(USART2, "\r\nDevice lost.\r\n"); 
            }
        }

        // Send a ping 
        nrf24l01_send_payload(trackers.write_buff, 4); 
    }

    // Check if any data has been received 
    if (nrf24l01_data_ready_status())
    {
        uart_sendstring(USART2, "\r\nData!\r\n"); 
        // Data has been received. Read the payload from the device FIFO and check to see if 
        // it's the ping response. If it is then set to ping flag to indicate the device was 
        // seen. 
        nrf24l01_receive_payload(trackers.read_buff); 
        if (str_compare((char *)trackers.response, (char *)trackers.read_buff, BYTE_0)) 
        {
            uart_sendstring(USART2, "\r\nPing!\r\n"); 
            trackers.ping = SET_BIT; 
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
    if (nrf24l01_data_ready_status())
    {
        uart_sendstring(USART2, "\r\nData!\r\n"); 
        // Data has been received. Read the payload from the device FIFO and check to see if 
        // it's a ping. If it is then send back a ping response. 
        nrf24l01_receive_payload(trackers.read_buff); 
        if (str_compare((char *)trackers.message, (char *)trackers.read_buff, BYTE_0)) 
        {
            uart_sendstring(USART2, "\r\nPing!\r\n"); 
            nrf24l01_send_payload(trackers.write_buff, 12); 
        }
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
