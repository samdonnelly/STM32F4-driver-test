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

// Configuration 
#define NRF24L01_RF_FREQ 10      // 2400 MHz + this value --> communication frequency (MHz) 

// Conditional compilation 
// The test code has code for two RF modules meant for use on separate controllers since 
// they're made for wireless communication between devices. The following macro ensures only 
// one controller code or the other is enabled. This can be toggled to easily flash the 
// correct code to a controller. 
#define NRF24L01_DEV1_CODE 1     // Device code selection 
#define NRF24L01_HEARTBEAT 1     // Heartbeat test code 
#define NRF24L01_MULTI_SPI 0     // Test code using SD card for same SPI bus on multiple pins 
#define NRF24L01_RC 0            // Remote control test code 

// Heartbeat 
#define NRF24L01_HB_PERIOD 1000000   // Time between heartbeat checks (us) 

// Multi-SPI 


// RC 
#define NRF24L01_LEFT_MOTOR 0x4C   // "L" character that indicates left motor 
#define NRF24L01_RIGHT_MOTOR 0x52  // "R" character that indicates right motor 
#define NRF24L01_FWD_THRUST 0x2B   // "+" character that indicates forward thrust 
#define NRF24L01_REV_THRUST 0x2D   // "-" character that indicates reverse thrust 

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
