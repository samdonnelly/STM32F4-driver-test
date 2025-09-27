/**
 * @file hw125_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 driver test interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW125_TEST_H_
#define _HW125_TEST_H_

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
 * @brief HW125 setup code 
 */
void hw125_test_init(void); 


/**
 * @brief HW125 test code 
 */
void hw125_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HW125_TEST_H_ 
