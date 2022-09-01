/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_app.h"

//=======================================================================================


// User application 
void user_app()
{
    // Looped code here
    // Call application or test code from here 

    // SAM-M8Q GPS 
    m8q_test_app(); 

    // MPU6050 
    // mpu6050_test_app(); 
}
