/**
 * @file i2c_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _I2C_TEST_H_
#define _I2C_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief I2C setup code 
 * 
 */
void i2c_test_init(void); 


/**
 * @brief I2C test code 
 * 
 */
void i2c_test_app(void); 

//=======================================================================================

#endif  // _I2C_TEST_H_ 
