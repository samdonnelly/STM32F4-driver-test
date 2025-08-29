/**
 * @file pose_estimate_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Pose estimation test interface 
 * 
 * @version 0.1
 * @date 2025-08-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _POSE_ESTIMATE_TEST_H_
#define _POSE_ESTIMATE_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "includes_cpp_drivers.h"

//=======================================================================================


//=======================================================================================
// Classes 

class PoseEstimate final
{
public: 
    
    /**
     * @brief Constructor 
     */
    PoseEstimate();

    /**
     * @brief Destructor 
     */
    ~PoseEstimate() = default;

    // Delete copy constructor and assignment operator
    PoseEstimate(const PoseEstimate &) = delete;
    PoseEstimate &operator=(const PoseEstimate &) = delete;

    // Delete move constructor and assignment operator
    PoseEstimate(PoseEstimate &&) = delete;
    PoseEstimate &operator=(PoseEstimate &&) = delete;

    /**
     * @brief Pose estimate test initialization and setup code 
     */
    void TestInit(void); 

    /**
     * @brief Pose estimate test application code 
     */
    void TestApp(void); 

private: 

    /**
     * @brief Check for driver faults, halt program if they exist 
     */
    void DeviceFaultCheck(void);

    /**
     * @brief Find the global position of the system 
     */
    void PoseCalcs(void);

    /**
     * @brief Output the determined position of the system for the user to see 
     */
    void PoseDisplay(void);

    // Peripherals 
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_calc, *tim_display;

    // MPU-6050 IMU data 
    device_number_t device_num; 
    uint8_t imu_st_result; 
    MPU6050_STATUS imu_status; 
    std::array<float, NUM_AXES> accel, gyro;

    // LSM303AGR magnetometer data 
    LSM303AGR_STATUS mag_status;
    std::array<float, NUM_AXES> mag;

    // M8Q GPS data 
    M8Q_STATUS gps_status;

    // Calculations 
    MadgwickFilter madgwick_filter;
    NavCalcs nav_calcs;
    uint8_t kalman_update;
    float lat, lon, alt;
    std::array<float, NUM_AXES> accel_ned, vel;
};

extern PoseEstimate pose_estimate;

//=======================================================================================

#endif   // _POSE_ESTIMATE_TEST_H_
