/**
 * @file freertos_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS practice interface 
 * 
 * @details The examples/tests implemented here are from the "Introduction to RTOS" video 
 *          playlist by DigiKey: 
 *          https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz 
 * 
 * @version 0.1
 * @date 2024-02-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _FREERTOS_TEST_H_ 
#define _FREERTOS_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif   // _FREERTOS_TEST_H_
