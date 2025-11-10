/**
 * @file hd44780u_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U LCD screen test code interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_TEST_H_
#define _HD44780U_TEST_H_

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
 * @brief HD44780U LCD sceen setup code 
 */
void hd44780u_test_init(void); 


/**
 * @brief HD44780U LCD sceen test code 
 */
void hd44780u_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HD44780U_TEST_H_ 
