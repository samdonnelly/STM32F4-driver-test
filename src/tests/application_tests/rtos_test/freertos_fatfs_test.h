/**
 * @file freertos_fatfs_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS with FatFs test interface 
 * 
 * @version 0.1
 * @date 2025-11-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _FREERTOS_FATFS_TEST_H_
#define _FREERTOS_FATFS_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include <array>

//=======================================================================================


//=======================================================================================
// Global data 

constexpr uint16_t frff_buff_size = 250;

//=======================================================================================


//=======================================================================================
// Classes 

class FreeRTOSFatFsTest final
{
public:

    // Constructor 
    FreeRTOSFatFsTest();

    // Destructor 
    ~FreeRTOSFatFsTest() = default;

    // Delete copy constructor and assignment operator
    FreeRTOSFatFsTest(const FreeRTOSFatFsTest &) = delete;
    FreeRTOSFatFsTest &operator=(const FreeRTOSFatFsTest &) = delete;

    // Delete move constructor and assignment operator
    FreeRTOSFatFsTest(FreeRTOSFatFsTest &&) = delete;
    FreeRTOSFatFsTest &operator=(FreeRTOSFatFsTest &&) = delete;

    /**
     * @brief FreeRTOS with FatFs test initialization and setup code 
     */
    void TestInit(void);

    /**
     * @brief FreeRTOS with FatFs test application code 
     */
    void TestApp(void);

private:

    // Peripherals 
    TIM_TypeDef *timer;
    USART_TypeDef *uart;
    SPI_TypeDef *spi;
    GPIO_TypeDef *spi_gpio;
    DMA_Stream_TypeDef *dma_stream;

    // Serial interface data 
    std::array<uint8_t, frff_buff_size> cb;              // Circular buffer populated by DMA 
    cb_index_t cb_index;                                 // Circular buffer indexing info 
    dma_index_t dma_index;                               // DMA transfer indexing info 
    std::array<uint8_t, frff_buff_size> data_in_buff;    // Buffer that stores latest UART input 
    std::array<uint8_t, frff_buff_size> data_out_buff;   // Buffer that stores outgoing data 
    DWORD data_in_num;                                   // Number fetched from data input 

    // File variables 
    FATFS file_sys;                                      // File system 
    FIL file;                                            // File 
    FRESULT fresult;                                     // Store the result of each operation 
    UINT br, bw;                                         // Stores f_read and f_write byte counters 
    DIR dj;                                              // Directory object 
    FILINFO fno;                                         // File information 
};

extern FreeRTOSFatFsTest freertos_fatfs;

//=======================================================================================

#endif   // _FREERTOS_FATFS_TEST_H_ 
