/**
 * @file mpu6050_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_TEST_H_
#define _MPU6050_TEST_H_

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
 * @brief MPU-6050 test initialization and setup code 
 */
void mpu6050_test_init(void); 


/**
 * @brief MPU-6050 test application code 
 */
void mpu6050_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _MPU6050_TEST_H_ 
