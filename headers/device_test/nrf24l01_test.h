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

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief nRF24L01 setup code 
 */
void nrf24l01_test_init(void); 


/**
 * @brief nRF24L01 test code 
 */
void nrf24l01_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_TEST_H_ 
