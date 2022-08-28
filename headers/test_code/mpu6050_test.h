/**
 * @file mpu6050_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_TEST_H_
#define _MPU6050_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief MPU-6050 setup code 
 * 
 */
void mpu6050_test_init(void); 


/**
 * @brief MPU-6050 test code 
 * 
 */
void mpu6050_test_app(void); 

//=======================================================================================

#endif  // _MPU6050_TEST_H_ 
