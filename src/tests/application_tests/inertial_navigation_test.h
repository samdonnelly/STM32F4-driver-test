/**
 * @file inertial_navigation_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Inertial navigation test interface 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _INERTIAL_NAVIGATION_TEST_H_ 
#define _INERTIAL_NAVIGATION_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Prototypes 

class InertialNavigationTest final
{
public: 
    
    /**
     * @brief Constructor 
     */
    InertialNavigationTest();

    /**
     * @brief Destructor 
     */
    ~InertialNavigationTest() = default;

    // Delete copy constructor and assignment operator
    InertialNavigationTest(const InertialNavigationTest &) = delete;
    InertialNavigationTest &operator=(const InertialNavigationTest &) = delete;

    // Delete move constructor and assignment operator
    InertialNavigationTest(InertialNavigationTest &&) = delete;
    InertialNavigationTest &operator=(InertialNavigationTest &&) = delete;

    /**
     * @brief Dead reckoning test initialization and setup code 
     */
    void TestInit(void); 

    /**
     * @brief Dead reckoning test application code 
     */
    void TestApp(void); 

private: 

    /**
     * @brief Fault state for the IMU 
     */
    void IMUFault(void);

    /**
     * @brief Estimate the heading, velocity and position using data from the IMU 
     */
    void InertialNavCalcs(void);
    
    // Peripherals 
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_periodic; 

    // IMU data 
    device_number_t device_num; 
    uint8_t st_result; 
    MPU6050_STATUS status; 
};

extern InertialNavigationTest inertial_navigation; 

//=======================================================================================

#endif   // _INERTIAL_NAVIGATION_TEST_H_ 
