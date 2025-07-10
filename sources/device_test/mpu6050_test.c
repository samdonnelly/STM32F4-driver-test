/**
 * @file mpu6050_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. 
 *          
 *          Procedure 
 *          - 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "mpu6050_test.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Data 
#define MPU6050_DEV1_STBY_MASK 0x00         // Device 1 axis standby status mask 
#define MPU6050_DEV2_STBY_MASK 0x00         // Device 2 axis standby status mask 
#define MPU6050_SMPLRT_DIV 0                // Sample Rate Divider 

// Driver test 
#define MPU6050_DRIVER_LOOP_DELAY 100       // Delay (blocking) between code loops (ms) 
#define MPU6050_DRIVER_ST_DELAY 10          // Delay (blocking) after self test (ms) 


#define MPU6050_TEST_DATA_OUTPUT_SPACES 2   // Spaces between data when outputing 

//=======================================================================================


//=======================================================================================
// Setup code 

void mpu6050_test_init()
{
    //===================================================
    // Peripheral initialization 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR_BIT, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize I2C1
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    //===================================================

    //===================================================
    // Accelerometer initialization 
    
    // Initialize the accelerometer 
    mpu6050_init(
        DEVICE_ONE, 
        I2C1, 
        MPU6050_ADDR_1,
        MPU6050_DEV1_STBY_MASK, 
        MPU6050_DLPF_CFG_1,
        MPU6050_SMPLRT_DIV,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    if (!mpu6050_get_status(DEVICE_ONE)) 
    {
        uart_send_str(USART2, "Device seen\r\n");
    }
    else 
    {
        uart_send_str(USART2, "Device not seen\r\n");
    }

#if MPU6050_SECOND_DEVICE 

    // Initialize the second accelerometer 
    mpu6050_init(
        DEVICE_TWO, 
        I2C1, 
        MPU6050_ADDR_2,
        MPU6050_DEV2_STBY_MASK, 
        MPU6050_DLPF_CFG_1,
        MPU6050_SMPLRT_DIV,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    if (!mpu6050_get_status(DEVICE_TWO)) 
    {
        uart_send_str(USART2, "Second device seen\r\n");
    }
    else 
    {
        uart_send_str(USART2, "Second device not seen\r\n");
    }

#endif   // MPU6050_SECOND_DEVICE 

    //===================================================

    //===================================================
    // Setup 

    // MPU6050 self-test 
    uint8_t mpu_self_test_result = mpu6050_self_test(DEVICE_ONE);
    uart_send_str(USART2, "MPU6050 Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Provide time for the device to update data so self-test data is not used elsewhere 
    tim_delay_ms(TIM9, MPU6050_DRIVER_ST_DELAY); 

    // Calibrate the device 
    mpu6050_calibrate(DEVICE_ONE); 
    
    
#if MPU6050_INT_PIN 

    // Set up the INT pin 
    mpu6050_int_pin_init(GPIOC, PIN_11); 

#endif   // MPU6050_INT_PIN 


#if MPU6050_SECOND_DEVICE 

    // MPU6050 self-test - second device 
    mpu_self_test_result = mpu6050_self_test(DEVICE_TWO);
    uart_send_str(USART2, "MPU6050 Second Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Provide time for the device to update data so self-test data is not used elsewhere 
    tim_delay_ms(TIM9, MPU6050_DRIVER_ST_DELAY); 

    // Calibrate the device 
    mpu6050_calibrate(DEVICE_TWO); 

#endif   // MPU6050_SECOND_DEVICE 

    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void mpu6050_test_app()
{
    static int16_t mpu6050_temp_sensor; 
    static float mpu6050_accel[NUM_AXES]; 
    static float mpu6050_gyro[NUM_AXES]; 
    // char output_buffer[100]; 

    // Update the accelerometer, temperature and gyroscope readings for device one 
    mpu6050_update(DEVICE_ONE); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_ONE) * SCALE_100); 
    mpu6050_get_accel_axis_gs(DEVICE_ONE, mpu6050_accel); 
    mpu6050_get_gyro_axis_rate(DEVICE_ONE, mpu6050_gyro); 

    // Change the below to use snprintf instead. 
    // snprintf(output_buffer, 
    //          100, 
    //          "temp1 = %d ax1 = %d ay1 = %d az1 = %d gx1 = %d gy1 = %d gz1 = %d", 
    //          mpu6050_temp_sensor, 
    //          (int16_t)(mpu6050_accel[X_AXIS] * SCALE_100), 
    //          (int16_t)(mpu6050_accel[Y_AXIS] * SCALE_100), 
    //          (int16_t)(mpu6050_accel[Z_AXIS] * SCALE_100), 
    //          (int16_t)(mpu6050_gyro[X_AXIS] * SCALE_100), 
    //          (int16_t)(mpu6050_gyro[Y_AXIS] * SCALE_100), 
    //          (int16_t)(mpu6050_gyro[Z_AXIS] * SCALE_100)); 

    // Display the first device results - values are scaled to remove decimal 
    uart_send_str(USART2, "temp1 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "ax1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "ay1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "az1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gx1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gy1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gz1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);


#if MPU6050_SECOND_DEVICE 

    // Display second device results on a new line 
    uart_send_new_line(USART2);

    // Update the accelerometer, temperature and gyroscope readings for device two 
    mpu6050_update(DEVICE_TWO); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_TWO) * SCALE_100); 
    mpu6050_get_accel_axis_gs(DEVICE_TWO, mpu6050_accel); 
    mpu6050_get_gyro_axis_rate(DEVICE_TWO, mpu6050_gyro); 

    // Display the second device results 
    uart_send_str(USART2, "temp2 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "ax2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "ay2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "az2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gx2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gy2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    uart_send_str(USART2, "gz2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, MPU6050_TEST_DATA_OUTPUT_SPACES);

    // Go up a line in the terminal to overwrite old data 
    uart_send_str(USART2, "\033[1A"); 

#endif   // MPU6050_SECOND_DEVICE 

    // Delay 
    tim_delay_ms(TIM9, MPU6050_DRIVER_LOOP_DELAY);

    // Go to a the start of the line in the terminal 
    uart_send_str(USART2, "\r"); 
}

//=======================================================================================
