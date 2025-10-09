/**
 * @file hc05_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 test code interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HC05_TEST_H_
#define _HC05_TEST_H_

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
 * @brief HC05 setup code 
 */
void hc05_test_init(void); 


/**
 * @brief HC05 test code 
 */
void hc05_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HC05_TEST_H_ 
