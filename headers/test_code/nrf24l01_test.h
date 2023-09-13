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
#define NRF24L01_MULTI_SPI 0     // Test code using SD card for same SPI bus on multiple pins 
#define NRF24L01_HEARTBEAT 0     // Heartbeat test code 
// The test code has code for two RF modules meant for use on separate controllers since 
// they're made for wireless communication between devices. The following macro ensures only 
// one controller code or the other is enabled. This can be toggled to easily flash the 
// correct code to a controller. 
#define NRF24L01_DEV1_CODE 1     // Device code selection 

// Configuration 
#define NRF24L01_RF_FREQ 50      // 2400 MHz + this value --> communication frequency (MHz) 

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
