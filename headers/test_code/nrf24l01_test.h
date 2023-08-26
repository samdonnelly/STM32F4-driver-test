/**
 * @file nrf24l01_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module test code header 
 * 
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NRF24L01_TEST_H_ 
#define _NRF24L01_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
// Test RF module and SD card on same SPI bus but multiple SPI pins 
#define NRF24L01_DEV1_EN 0       // Device 1 enable 
#define NRF24L01_DEV2_EN 0       // Device 2 enable 
#define NRF24L01_HEARTBEAT 0     // Heartbeat test code 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Setup code 
 */
void nrf24l01_test_init(void); 


/**
 * @brief Test code 
 */
void nrf24l01_test_app(void); 

//=======================================================================================

#endif   // _NRF24L01_TEST_H_ 
