/**
 * @file switch_debounce_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Switch debounce test interface 
 * 
 * @version 0.1
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _SWITCH_DEBOUNCE_TEST_H_ 
#define _SWITCH_DEBOUNCE_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Switch debounce test setup code 
 */
void switch_debounce_test_init(void); 


/**
 * @brief Switch debounce test code 
 */
void switch_debounce_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _SWITCH_DEBOUNCE_TEST_H_
