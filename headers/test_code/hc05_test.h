/**
 * @file hc05_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC-05 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HC05_TEST_H_
#define _HC05_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

#include "state_machine_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define HC05_CONTROLLER_TEST 0     // For switching between driver and controller testing 
#define HC05_NUM_USER_CMDS 10      // Number of defined user commands for controller test 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HC-05 setup code 
 * 
 */
void hc05_test_init(void); 


/**
 * @brief HC-05 test code 
 * 
 */
void hc05_test_app(void); 

//=======================================================================================

#endif  // _HC05_TEST_H_ 
