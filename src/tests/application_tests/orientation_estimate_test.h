/**
 * @file orientation_estimate_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Orientation estimate test interface 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _ORIENTATION_ESTIMATE_TEST_H_ 
#define _ORIENTATION_ESTIMATE_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "includes_cpp_drivers.h"
#include <array>

//=======================================================================================


//=======================================================================================
// Prototypes 

class OrientationEstimateTest final
{
public: 
    
    /**
     * @brief Constructor 
     */
    OrientationEstimateTest();

    /**
     * @brief Destructor 
     */
    ~OrientationEstimateTest() = default;

    // Delete copy constructor and assignment operator
    OrientationEstimateTest(const OrientationEstimateTest &) = delete;
    OrientationEstimateTest &operator=(const OrientationEstimateTest &) = delete;

    // Delete move constructor and assignment operator
    OrientationEstimateTest(OrientationEstimateTest &&) = delete;
    OrientationEstimateTest &operator=(OrientationEstimateTest &&) = delete;

    /**
     * @brief Orientation estimate test initialization and setup code 
     */
    void TestInit(void); 

    /**
     * @brief Orientation estimate test application code 
     */
    void TestApp(void); 

private: 

    /**
     * @brief Check for driver faults, halt program if they exist 
     */
    void IMUFaultCheck(void);

    /**
     * @brief Estimate the orientation of system in the Earth frame (roll, pitch, yaw) 
     */
    void OrientationCalcs(void);

    /**
     * @brief Output the orientation for the use to see 
     */
    void OrientationDisplay(void);
    
    // Peripherals 
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_periodic;

    // Timing 
    uint8_t display_counter;

    // MPU-6050 IMU data 
    device_number_t device_num; 
    uint8_t imu_st_result; 
    MPU6050_STATUS imu_status; 
    std::array<int16_t, NUM_AXES> accel_raw, gyro_raw;
    std::array<float, NUM_AXES> accel, gyro;

    // LSM303AGR magnetometer data 
    LSM303AGR_STATUS mag_status;
    std::array<int16_t, NUM_AXES> mag_raw, mag;
    std::array<float, NUM_AXES> magf;

    // Calculations 
    MadgwickFilter madgwick_filter;
    float roll, pitch, yaw; 
};

extern OrientationEstimateTest orientation_estimate; 

//=======================================================================================

#endif   // _ORIENTATION_ESTIMATE_TEST_H_ 
