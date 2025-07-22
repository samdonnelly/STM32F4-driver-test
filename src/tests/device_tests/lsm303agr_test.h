/**
 * @file lsm303agr_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code interface 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _LSM303AGR_TEST_H_ 
#define _LSM303AGR_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief LSM303AGR setup code
 */
void lsm303agr_test_init(void); 


/**
 * @brief LSM303AGR test code 
 */
void lsm303agr_test_app(void); 

//=======================================================================================

#endif   // _LSM303AGR_TEST_H_ 
