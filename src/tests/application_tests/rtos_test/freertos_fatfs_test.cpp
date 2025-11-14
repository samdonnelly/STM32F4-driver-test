/**
 * @file freertos_fatfs_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS with FatFs test 
 * 
 * @version 0.1
 * @date 2025-11-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "freertos_fatfs_test.h"
#include "stm32f4xx_it.h"

//=======================================================================================


//=======================================================================================
// Test data 

FreeRTOSFatFsTest freertos_fatfs;

//=======================================================================================


//=======================================================================================
// Setup 

FreeRTOSFatFsTest::FreeRTOSFatFsTest()
    : timer(TIM9),
      uart(USART2),
      spi(SPI2),
      spi_gpio(GPIOB),
      dma_stream(DMA1_Stream5) {}


void FreeRTOSFatFsTest::TestInit(void)
{
    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init();
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        timer, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE);
    tim_enable(timer);

    //==================================================

    //==================================================
    // UART 
    
    // UART2 init - Serial terminal 
    uart_init(
        uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE);
        
    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
    uart_interrupt_init(
        uart, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE);
    
    //==================================================

    //==================================================
    // SPI 
    
    // SPI and slave select pin for SD card 
    spi_init(
        spi, 
        spi_gpio,            // SCK pin GPIO port 
        PIN_10,              // SCK pin 
        spi_gpio,            // Data (MISO/MOSI) pin GPIO port 
        PIN_14,              // MISO pin 
        PIN_15,              // MOSI pin 
        SPI_BR_FPCLK_8, 
        SPI_CLOCK_MODE_0);
    spi_ss_init(spi_gpio, PIN_12);
    
    //==================================================

    //==================================================
    // DMA 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        dma_stream, 
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
        dma_stream, 
        (uint32_t)(&uart->DR), 
        (uint32_t)cb.data(), 
        (uint32_t)nullptr, 
        frff_buff_size);
        
    // Enable DMA streams 
    dma_stream_enable(dma_stream);   // UART2 - Serial terminal 
    
    //==================================================

    //==================================================
    // Interrupts 

    // Initialize interrupt handler flags 
    int_handler_init();

    // Enable the interrupt handlers 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0);   // UART2 - Serial terminal (user input) 

    //==================================================

    //==================================================
    // SD card init 

    // SD card user initialization 
    sd_user_init(spi, spi_gpio, timer, GPIOX_PIN_12);
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void FreeRTOSFatFsTest::TestApp(void)
{
    // 
}

//=======================================================================================
