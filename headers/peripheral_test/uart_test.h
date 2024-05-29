/**
 * @file uart_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief UART test code interface 
 * 
 * @version 0.1
 * @date 2023-09-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _UART_TEST_H_ 
#define _UART_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief UART test setup code 
 */
void uart_test_init(void); 


/**
 * @brief UART test application code 
 */
void uart_test_app(void); 

//=======================================================================================

#endif   // _UART_TEST_H_ 
