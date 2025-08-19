/**
 * @file orientation_estimate_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Orientation estimate test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * 1 MPU-6050 IMU 
 *            * 1 LSM303AGR magnetometer 
 *            * NOTE: The IMU and magnetometer must be fixed relative to one another so 
 *                    the orientation of the "system" or "object" can be determined. This 
 *                    could be achieved by mounting them to the same board. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A timer is configured to create a periodic interrupt which controls when 
 *              to read and output device data. 
 *          - UART 
 *            * UART is configured to provide a serial terminal output both for device 
 *              data and driver status faults. 
 *          - I2C 
 *            * I2C is configured to communicate with the MPU-6050 and LSM303AGR devices. 
 *            * It is set to run in standard mode (SM) where SCL runs at 100kHz which 
 *              should handle a typical transaction with the MPU-6050 in ~1ms. 
 *          - Interrupts 
 *            * An interrupt is configured for the timer to create a periodic interrupt 
 *              to control when to read and output device data. 
 *          - MPU-6050 
 *            * The accelerometer updates/outputs at a rate of 1kHz (can't be adjusted). 
 *            * The gyroscope is set to update/output at a rate of 1kHz. The DLPF is 
 *              enabled and the SMPLRT_DIV register is set to 0 (see datasheet). 
 *          - LSM303AGR 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. 
 *          
 *          Procedure 
 *          - The IMU and magnetometer are assumed to be part of the same rigid body system 
 *            and are read from periodically. The read data is fed into a Madgwick filter 
 *            which is used to estimate the orientation of the system. The orientation is 
 *            displayed to the serial terminal periodically for the user to see. 
 *            
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "orientation_estimate_test.h"
#include "device_config.h"
#include "stm32f4xx_it.h"

//=======================================================================================


//=======================================================================================
// To Do: 
// - What are the unit required to be fed to the Madgwick filter? 
// - How will we handle getting magnetometer data as a float? s
//=======================================================================================


//=======================================================================================
// Test data 

OrientationEstimateTest orientation_estimate; 

static constexpr uint16_t interrupt_counter = 0x01F4;   // ARR=500, (500 counts)*(100us/count) = 50ms = 0.05s 
static constexpr uint8_t max_msg_len = 100;             // Max length of output message 
static constexpr uint8_t display_timer = 5;             // Interrupt count that triggers a data display update 

static constexpr float madgwick_B = 0.1;                // Correction weight 
static constexpr float madgwick_dt = 0.05;              // Time between samples/calculations (seconds) 

//=======================================================================================


//=======================================================================================
// Setup 

OrientationEstimateTest::OrientationEstimateTest()
    : uart(USART2),
      i2c(I2C1),
      tim_periodic(TIM10),
      display_counter(CLEAR),
      device_num(DEVICE_ONE),
      imu_st_result(CLEAR),
      imu_status(MPU6050_OK), 
      accel_raw{}, gyro_raw{}, accel{}, gyro{},
      mag_status(LSM303AGR_OK),
      mag_raw{}, mag{}, magf{},
      madgwick_filter(madgwick_B, madgwick_dt)
{
}

void OrientationEstimateTest::TestInit(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        tim_periodic, 
        TIM_84MHZ_100US_PSC, 
        interrupt_counter, 
        TIM_UP_INT_ENABLE); 
    tim_enable(tim_periodic); 

    // UART - serial terminal output 
    uart_init(
        uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR_BIT, 
        UART_FRAC_42_115200, 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize I2C1
    i2c_init(
        i2c, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42); 

    // Initialize interrupt handler flags and enable the periodic timer interrupt handler 
    int_handler_init(); 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_0); 
    
    // Initialization the MPU-6050, run self-test and set the axis offsets 
    imu_status |= mpu6050_init(
        device_num, 
        i2c, 
        MPU6050_ADDR_1,
        standby_mask, 
        MPU6050_DLPF_CFG_1,
        sample_rate_divider,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);
    imu_status |= mpu6050_self_test(device_num, &imu_st_result); 
    imu_status |= mpu6050_set_offsets(device_num, accel_offsets, gyro_offsets); 

    // Initialize the LSM303AGR and set the hard and soft-iron calibration values 
    mag_status |= lsm303agr_m_init(
        I2C1, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
    lsm303agr_m_calibration_set(lsm303agr_hi_offset, lsm303agr_sid_values, lsm303agr_sio_values); 

    IMUFaultCheck(); 
}

//=======================================================================================


//=======================================================================================
// Application 

void OrientationEstimateTest::TestApp(void)
{
    // Periodically update IMU data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Read the latest IMU and magnetometer data 
        imu_status |= mpu6050_update(device_num); 
        mag_status |= lsm303agr_m_update(); 

        // Check for driver faults 
        IMUFaultCheck(); 

        // Get the latest accelerometer, gyroscope and magnetometer data 
        mpu6050_get_accel_axis(device_num, accel_raw.data());   // Raw 
        mpu6050_get_gyro_axis(device_num, gyro_raw.data());     // Raw 
        mpu6050_get_accel_axis_gs(device_num, accel.data());    // g's 
        mpu6050_get_gyro_axis_rate(device_num, gyro.data());    // deg/s 
        lsm303agr_m_get_axis(mag_raw.data());                   // Uncalibrate - milligauss 
        lsm303agr_m_get_calibrated_axis(mag.data());            // Calibrated - milligauss 

        // Perform inertial navigation calcs 
        OrientationCalcs();

        // Output the orientation (doesn't need to be as frequent as the calculation) 
        if (++display_counter >= display_timer)
        {
            display_counter = CLEAR;
            OrientationDisplay();
        }
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

void OrientationEstimateTest::IMUFaultCheck(void)
{
    if ((imu_status != MPU6050_OK) || (mag_status != LSM303AGR_OK))
    {
        char fault_msg[max_msg_len]; 
        snprintf(
            fault_msg, 
            max_msg_len, 
            "\r\nIMU ST: %u"
            "\r\nIMU Fault Code: %lu"
            "\r\nMag Fault Code: %lu", 
            imu_st_result,
            imu_status,
            mag_status); 
        uart_send_str(uart, fault_msg); 
        tim_disable(tim_periodic); 
        while(TRUE); 
    }
}


// Estimate the orientation of system in the Earth frame (roll, pitch, yaw) 
void OrientationEstimateTest::OrientationCalcs(void)
{
    madgwick_filter.Madgwick(gyro, accel, magf);
    roll = madgwick_filter.GetRoll();
    pitch = madgwick_filter.GetPitch();
    yaw = madgwick_filter.GetYaw();

    // Apply magnetic declination to get yaw to true north 

    // Cap/bound angles as needed to keep them within range 
}


// Output the orientation for the use to see 
void OrientationEstimateTest::OrientationDisplay(void)
{
    // Move the cursor in the serial terminal up to overwrite the old data 
    uart_cursor_move(uart, UART_CURSOR_UP, 2);

    // Format and output the scaled orientation data 
    char orientation_msg[max_msg_len];
    snprintf(
        orientation_msg, 
        max_msg_len, 
        "\r\nRoll (deg*100): %d"
        "\r\nPitch (deg*100): %d"
        "\r\nYaw (deg*100): %d",
        (int16_t)(roll * SCALE_100),
        (int16_t)(pitch * SCALE_100),
        (int16_t)(yaw * SCALE_100));
    uart_send_str(uart, orientation_msg);
}

//=======================================================================================
