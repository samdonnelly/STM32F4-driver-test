/**
 * @file freertos_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS setup and test interface 
 * 
 * @details Basic applications of FreeRTOS features are written in this test to show its 
 *          capabilities. 
 * 
 * @version 0.1
 * @date 2024-02-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _FREERTOS_TEST_H_ 
#define _FREERTOS_TEST_H_ 

//=======================================================================================
// Includes 
//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief FreeRTOS test setup code 
 */
void freertos_test_init(void); 


/**
 * @brief FreeRTOS test code 
 */
void freertos_test_app(void); 

//=======================================================================================

#endif   // _FREERTOS_TEST_H_
