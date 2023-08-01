/**
 * @file lsm303agr_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code header 
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
// Macros 

// Conditional compilation 
#define LSM303AGR_TEST_AXIS 0             // Magnetometer axis values 
#define LSM303AGR_TEST_HEADING 1          // Heading (compass) test 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Setup code
 */
void lsm303agr_test_init(void); 


/**
 * @brief Test code 
 */
void lsm303agr_test_app(void); 

//=======================================================================================

#endif   // _LSM303AGR_TEST_H_ 
