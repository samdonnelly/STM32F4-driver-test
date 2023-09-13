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
// Notes 

// Notes 
// - With two devices, they should both remain in RX mode to wait for data until they 
//   need to be used for sending data. 

// Tests 
// - Heartbeat / handshake between two devices 

//=======================================================================================


//=======================================================================================
// Function prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 
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

    //===================================================
    // Initialize SPI 

    // SPI for the RF module 
    spi_init(
        SPI2, 
        GPIOB,   // SCK pin GPIO port 
        PIN_10,  // SCK pin 
        GPIOC,   // Data (MISO/MOSI) pin GPIO port 
        PIN_2,   // MISO pin 
        PIN_3,   // MOSI pin 
        SPI_BR_FPCLK_16, 
        SPI_CLOCK_MODE_0); 

#if NRF24L01_MULTI_SPI && NRF24L01_DEV1_CODE 

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
        GPIOC, 
        GPIOX_PIN_1, 
        GPIOC, 
        GPIOX_PIN_0, 
        NRF24L01_DR_250KBPS, 
        NRF24L01_RF_FREQ); 

    //===================================================

    //===================================================
    // Initialize SD card --> for testing multiple SPI pins on the same SPI bus 

#if NRF24L01_MULTI_SPI && NRF24L01_DEV1_CODE 

    // SD card user initialization 
    hw125_user_init(SPI2, GPIOB, GPIOX_PIN_12); 

#endif   // NRF24L01_MULTI_SPI 
    
    //===================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void nrf24l01_test_app(void)
{
#if NRF24L01_DEV1_CODE   // Start of device 1 code 
    // 
#if NRF24L01_MULTI_SPI 
    // 
#endif   // NRF24L01_MULTI_SPI 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
