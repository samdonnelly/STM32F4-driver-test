/**
 * @file ibus_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2025-04-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _IBUS_TEST_H_ 
#define _IBUS_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Test code 

/**
 * @brief IBUS test setup code 
 */
void ibus_test_init(void); 


/**
 * @brief IBUS test application code 
 */
void ibus_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _IBUS_TEST_H_ 
