/**
 * @file pose_estimate_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Pose estimation test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * 1 MPU-6050 IMU 
 *            * 1 LSM303AGR magnetometer 
 *            * 1 SAM-M8Q GPS 
 *            * NOTE: The IMU and magnetometer must be fixed relative to one another so 
 *                    the orientation of the "system" or "object" can be determined. This 
 *                    could be achieved by mounting them to the same board. The GPS does 
 *                    not need to be fixed relative to the IMU and magnetometer but it 
 *                    must move with the system so position is known. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A timer is configured to create a periodic interrupt which controls when 
 *              to read data and calculate the orientation. 
 *            * A second timer is configured also as a periodic interrupt which is used 
 *              to control when data is output. This is done so the calculation frequency 
 *              can be updated without affecting the output rate. 
 *          - UART 
 *            * UART is configured to provide a serial terminal output both for device 
 *              data and driver status faults. 
 *          - I2C 
 *            * I2C is configured to communicate with the MPU-6050, LSM303AGR and SAM-M8Q 
 *              devices. 
 *            * It is set to run in standard mode (SM) where SCL runs at 100kHz which 
 *              should handle a typical transaction with the MPU-6050 in ~1ms. 
 *          - Interrupts 
 *            * Interrupts are configured for both timers to create periodic interrupts 
 *              that control when to read and output device data. 
 *          - MPU-6050 
 *            * The accelerometer updates/outputs at a rate of 1kHz (can't be adjusted). 
 *            * The gyroscope is set to update/output at a rate of 1kHz. The DLPF is 
 *              enabled and the SMPLRT_DIV register is set to 0 (see datasheet). 
 *          - LSM303AGR 
 *            * The magnetometer updates/outputs at a rate of 50Hz. This can be 
 *              configured to go as low as 10Hz. If the period for determining 
 *              orientation and position prediciton is chosen faster than this then 
 *              magnetometer data will be the same across multiple periods. 
 *          - SAM-M8Q
 *            * The GPS updates/outputs at a maximum of 1Hz. The kalman filter position 
 *              is only updated once this data comes in. 
 *          - User Config 
 *            * Madgwick filter correction weight (B) and the time between calculations 
 *              (dt). 
 *            * Magnetic declination for their location. 
 *            * Kalman filter process (accelerometer) and measurement (GPS) variance. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. 
 *          
 *          Procedure 
 *          - After configuring devices but before starting to determine position, the 
 *            code waits for a GPS position to be obtained so the initial location can 
 *            be set. 
 *          - Once the initial position is known, the code proceeds to determine the 
 *            systems orientation using accelerometer, gyroscope and magnetometer data 
 *            fed to a Madgwick filter which gets called periodically. Each orientation 
 *            update is followed by a prediciton of the systems global position using 
 *            a Kalman filter that takes in the systems acceleration in the NED frame. 
 *            Once GPS data becomes available, the Kalman filter update step is called 
 *            which fuses newly measured position and velocity data with the predicted 
 *            posiiton to provide the best possible estimate of the systems true 
 *            position and velocity. Determined position and velocity is displayed 
 *            periodically in the serial terminal for the user to see. 
 *          - The user is able to set Madgwick properties and well as magnetic 
 *            declination and Kalman filter variances. 
 * 
 * @version 0.1
 * @date 2025-08-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "pose_estimate_test.h"
#include "driver_test_config.h"
#include "stm32f4xx_it.h"

//=======================================================================================


//=======================================================================================
// Test data 

PoseEstimate pose_estimate;

// Counter update interupt ARR value --> (ARR counts)*(100us/count) = interrupt period (seconds) 
static const uint16_t int_calc_count = static_cast<uint16_t>(madgwick_dt * SCALE_10000);
static constexpr uint16_t int_display_count = 0x09C4;   // ARR=2500 

// Formatting 
static constexpr uint8_t max_msg_len = 200;      // Max length of output message 
static constexpr uint8_t num_output_lines = 6;   // Number of lines to move the cursor 

//=======================================================================================


//=======================================================================================
// Setup 

PoseEstimate::PoseEstimate()
    : uart(USART2),
      i2c(I2C1),
      tim_calc(TIM10),
      tim_display(TIM9),
      device_num(DEVICE_ONE),
      imu_st_result(CLEAR),
      imu_status(MPU6050_OK), 
      accel{}, gyro{},
      accel_var{ accel_variance[X_AXIS], accel_variance[Y_AXIS], accel_variance[Z_AXIS] },
      mag_status(LSM303AGR_OK),
      mag{},
      gps_status(M8Q_OK),
      madgwick_filter(madgwick_B, madgwick_dt),
      nav_calcs(),
      accel_ned{}, accel_ned_variance{},
      gps_pos(), gps_pos_var(), position(),
      gps_vel(), gps_vel_var(), velocity(),
      kalman_update(CLEAR_BIT)
{
    nav_calcs.SetTnOffset(magnetic_declination);
}


void PoseEstimate::TestInit(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer - read data and perform calculation 
    tim_9_to_11_counter_init(
        tim_calc, 
        TIM_84MHZ_100US_PSC, 
        int_calc_count, 
        TIM_UP_INT_ENABLE); 
    tim_enable(tim_calc); 

    // Periodic (counter update) interrupt timer - display data 
    tim_9_to_11_counter_init(
        tim_display, 
        TIM_84MHZ_100US_PSC, 
        int_display_count, 
        TIM_UP_INT_ENABLE); 
    tim_enable(tim_display); 

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
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_1); 
    
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
        i2c,
        LSM303AGR_M_ODR_50,
        LSM303AGR_M_MODE_CONT,
        LSM303AGR_CFG_DISABLE,
        LSM303AGR_CFG_DISABLE,
        LSM303AGR_CFG_DISABLE,
        LSM303AGR_CFG_DISABLE);
    lsm303agr_m_calibration_set(lsm303agr_hi_offset, lsm303agr_sid_values, lsm303agr_sio_values);

    // Initialize the M8Q GPS and set up the TX ready (data ready) pin 
    gps_status |= m8q_init(
        i2c,
        &m8q_config_pkt_0[0][0],
        M8Q_CONFIG_NUM_MSG_PKT_0,
        M8Q_CONFIG_MAX_LEN_PKT_0,
        CLEAR);
    gps_status |= m8q_txr_pin_init(GPIOC, PIN_11);

    // Check if there were any faults during driver setup 
    DeviceFaultCheck();

    // Wait until an initial GPS position is obtained before starting to estimate position. 
    WaitForGPS();
}

//=======================================================================================


//=======================================================================================
// Application 

void PoseEstimate::TestApp(void)
{
    // Periodically update device data and calculate the position 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR;

        // Read the latest data from the IMU, magnetometer and GPS (when available) 
        imu_status |= mpu6050_update(device_num);
        mag_status |= lsm303agr_m_update();
        if ((m8q_get_tx_ready() == GPIO_HIGH) && (m8q_get_position_navstat_lock() == TRUE))
        {
            gps_status |= m8q_read_data();
            kalman_update = SET_BIT;
        }

        // Check for driver faults 
        DeviceFaultCheck(); 

        // Perform position calculations 
        PoseCalcs();
    }

    // Periodically display the position 
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR;
        PoseDisplay();
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Check for driver faults, halt program if they exist 
void PoseEstimate::DeviceFaultCheck(void)
{
    if ((imu_status != MPU6050_OK) || (mag_status != LSM303AGR_OK) || (gps_status != M8Q_OK))
    {
        char fault_msg[max_msg_len]; 
        snprintf(
            fault_msg, 
            max_msg_len, 
            "\r\nIMU ST: %u"
            "\r\nIMU Fault Code: %lu"
            "\r\nMag Fault Code: %lu"
            "\r\nGPS Fault Code: %lu", 
            imu_st_result,
            imu_status,
            mag_status,
            gps_status); 
        uart_send_str(uart, fault_msg); 
        tim_disable(tim_calc); 
        tim_disable(tim_display); 
        while(true); 
    }
}


// Wait for an initial GPS connection 
void PoseEstimate::WaitForGPS(void)
{
    while (m8q_get_position_navstat_lock() == FALSE)
    {
        if (m8q_get_tx_ready() == GPIO_HIGH)
        {
            gps_status |= m8q_read_data();
        }

        if (handler_flags.tim1_brk_tim9_glbl_flag)
        {
            handler_flags.tim1_brk_tim9_glbl_flag = CLEAR;
            
            static uint8_t count = CLEAR;
            char load_symbol;
            char wait_msg[max_msg_len];

            switch(count++)
            {
                case 0:
                    load_symbol = SLASH_CHAR;
                    break;
                case 1:
                    load_symbol = MINUS_CHAR;
                    break;
                default:
                    load_symbol = BACKSLASH_CHAR;
                    count = CLEAR;
                    break;
            }
            
            snprintf(
                wait_msg, 
                max_msg_len, 
                "\rWaiting for GPS...%c", 
                load_symbol); 
            uart_send_str(uart, wait_msg);
        }
    }
    
    // Set the Kalman filter data now that we know the initial position 
    GetGPSData();
    nav_calcs.SetKalmanPoseData(madgwick_dt, gps_pos);
}


// Find the global position of the system 
void PoseEstimate::PoseCalcs(void)
{
    // Get the latest accelerometer, gyroscope and magnetometer data 
    mpu6050_get_accel_axis_gs(device_num, accel.data());   // g's 
    mpu6050_get_gyro_axis_rate(device_num, gyro.data());   // deg/s 
    lsm303agr_m_get_axis_cal_f(mag.data());                // mG 

    // Correct data as needed to be in the NWU orientation 
    mag[Y_AXIS] = -mag[Y_AXIS];

    // Run the Madgwick filter with new IMU data to determine the system orientation. 
    madgwick_filter.Madgwick(gyro, accel, mag);

    // Get the absolute acceleration in the NED frame relative to magnetic North then 
    // rotate it to be relative to true North. 
    madgwick_filter.GetAccelNED(accel, accel_ned);
    nav_calcs.TrueNorthEarthAccel(accel_ned[X_AXIS], accel_ned[Y_AXIS]);

    // Rotate the accelerometer uncertainty into the Earth frame relative to true North 
    // then make sure all values are positive. 
    madgwick_filter.BodyToEarth(accel_var, accel_ned_variance);
    nav_calcs.TrueNorthEarthAccel(accel_ned_variance[X_AXIS], accel_ned_variance[Y_AXIS]);

    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
	{
		if (accel_ned_variance[i] < ZERO)
		{
			accel_ned_variance[i] = -accel_ned_variance[i];
		}
	}

    // Prediction step of the Kalman filter. Use the latest accelerometer data to 
    // predict the position and velocity of the system. 
    nav_calcs.KalmanPosePredict(accel_ned, accel_ned_variance);

    // If new measured position and velocity data from the GPS device is available then 
    // we run the update step of the Kalman filter to provide the best estimate of true 
    // position and velocity. 
    if (kalman_update == SET_BIT)
    {
        kalman_update = CLEAR_BIT;
        GetGPSData();
        nav_calcs.KalmanPoseUpdate(gps_pos, gps_pos_var, gps_vel, gps_vel_var);
    }

    // Retreive the position and velocity determined by the Kalman filter. 
    nav_calcs.GetKalmanPose(position, velocity);
}


// Get GPS data 
void PoseEstimate::GetGPSData(void)
{
    gps_pos.lat = m8q_get_position_lat();                // deg 
    gps_pos.lon = m8q_get_position_lon();                // deg 
    gps_pos.alt = m8q_get_position_altref();             // m 
    gps_pos_var.lat = m8q_get_position_hacc();
    gps_pos_var.lon = gps_pos_var.lat;
    gps_pos_var.alt = m8q_get_position_vacc();

    gps_vel.sog = m8q_get_position_sog() / KPH_TO_MPS;   // m/s 
    gps_vel.cog = m8q_get_position_cog();                // deg 
    gps_vel.vvel = m8q_get_position_vvel();              // m/s 
    gps_vel_var.sog = gps_vel_variance[X_AXIS];
    gps_vel_var.cog = gps_vel_variance[Y_AXIS];
    gps_vel_var.vvel = gps_vel_variance[Z_AXIS];
}


// Output the determined position of the system for the user to see 
void PoseEstimate::PoseDisplay(void)
{
    // Move the cursor in the serial terminal up to overwrite the old data 
    uart_cursor_move(uart, UART_CURSOR_UP, num_output_lines);

    // Format and output the scaled orientation data 
    char position_msg[max_msg_len];
    snprintf(
        position_msg, 
        max_msg_len, 
        "Latitude (deg*1E6): %ld   \r\n"
        "Longitude (deg*1E6): %ld   \r\n"
        "Altitude (deg*1E3): %ld   \r\n"
        "SOG (m/s*1E3): %d   \r\n"
        "COG (deg*1E2): %d   \r\n"
        "vVel (m/s*1E3): %d   \r\n",
        static_cast<int32_t>(position.lat * SCALE_1E6F),
        static_cast<int32_t>(position.lon * SCALE_1E6F),
        static_cast<int32_t>(position.alt * SCALE_1000F),
        static_cast<int16_t>(velocity.sog * SCALE_1000F),
        static_cast<int16_t>(velocity.cog * SCALE_100F),
        static_cast<int16_t>(velocity.vvel * SCALE_1000F));
    uart_send_str(uart, position_msg);
}

//=======================================================================================
