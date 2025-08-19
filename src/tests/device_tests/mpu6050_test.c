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
 *            * 1 or 2 MPU-6050 IMUs connected to the STM32F4. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A timer is configured to create a periodic interrupt which controls when 
 *              to read and record device data. 
 *            * A second timer is configured also as a periodic interrupt which controls 
 *              when to update the data displayed to the user. 
 *          - UART 
 *            * UART is configured to provide a serial terminal output both for device 
 *              data and driver status faults. 
 *          - I2C 
 *            * I2C is configured to communicate with the MPU-6050 device. 
 *            * It is set to run in standard mode (SM) where SCL runs at 100kHz which 
 *              should handle a typical transaction with the MPU-6050 in ~1ms. 
 *          - Interrupts 
 *            * An interrupt is configured for the timer to create a periodic interrupt 
 *              to control when to read and output device data. 
 *          - MPU-6050 
 *            * The accelerometer updates/outputs at a rate of 1kHz (can't be adjusted). 
 *            * The gyroscope is set to update/output at a rate of 1kHz. The DLPF is 
 *              enabled and the SMPLRT_DIV register is set to 0 (see datasheet). 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. 
 *          
 *          Procedure 
 *          - During setup, the IMU will run a self test to check its axes. If this fails 
 *            then the status will be displayed and the code will be haulted. If successful 
 *            then the code proceeds to read and display raw and formatted IMU data 
 *            periodically in the serial terminal. If a fault occurs at any time then the 
 *            test will be haulted and the driver status displayed. 
 *          - New IMU data is read every 50ms and the new data gets summed into a buffer. 
 *            The displayed data is updated every 250ms at which point the summed data is 
 *            divided by the number of samples taken in the interval and that average value 
 *            is displayed to the user. This helps to smooth the data a bit without needing 
 *            a filter. The averaged raw values can be used to set the offsets for the 
 *            device in a given system. 
 *          - A second device can be enabled that does the same thing in parallel to the 
 *            the first device. 
 *            
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
#include "driver_test_config.h"
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Timing 
#define MPU6050_TEST_INT_READ_COUNT 0x01F4      // ARR=500, (500 counts)*(100us/count) = 50ms = 0.05s 
#define MPU6050_TEST_INT_DISPLAY_COUNT 0x09C4   // ARR=2500, (2500 counts)*(100us/count) = 250ms = 0.25s 

// Data output 
#define MPU6050_TEST_MAX_STR_SIZE 150       // Max output string size 
#define MPU6050_TEST_OUTPUT_LINES 16        // Number of output lines per IMU 

//=======================================================================================


//=======================================================================================
// Global data 

typedef struct mpu6050_test_imu_data_s
{
    device_number_t device_num; 
    MPU6050_STATUS status; 
    uint8_t st_result; 
    // Buffers to sum multiple data readings 
    int32_t temp_raw, accel_raw[NUM_AXES], gyro_raw[NUM_AXES]; 
    float temp, accel[NUM_AXES], gyro[NUM_AXES]; 
}
mpu6050_test_imu_data_t; 


typedef struct mpu6050_test_data_s 
{
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_read, *tim_display; 

    // Data output 
    uint8_t data_count;
    char output_raw[MPU6050_TEST_MAX_STR_SIZE]; 
    char output_formatted[MPU6050_TEST_MAX_STR_SIZE]; 
    uint8_t cursor_lines; 

    mpu6050_test_imu_data_t imu1; 

#if MPU6050_SECOND_DEVICE 

    mpu6050_test_imu_data_t imu2; 

#endif   // MPU6050_SECOND_DEVICE 
}
mpu6050_test_data_t; 

static mpu6050_test_data_t mpu6050_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Read and record IMU data 
 * 
 * @param imu_data : IMU data to use 
 */
void mpu6050_test_read_record(mpu6050_test_imu_data_t *imu_data);


/**
 * @brief Format and output IMU data 
 * 
 * @param imu_data : IMU data to use 
 */
void mpu6050_test_format_output(mpu6050_test_imu_data_t *imu_data); 


/**
 * @brief Output the driver status and stop program execution 
 */
void mpu6050_test_fault_state(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void mpu6050_test_init()
{
    mpu6050_data.uart = USART2; 
    mpu6050_data.i2c = I2C1; 
    mpu6050_data.tim_read = TIM10; 
    mpu6050_data.tim_display = TIM9; 
    mpu6050_data.data_count = CLEAR;
    memset((void *)mpu6050_data.output_raw, CLEAR, sizeof(mpu6050_data.output_raw)); 
    memset((void *)mpu6050_data.output_formatted, CLEAR, sizeof(mpu6050_data.output_formatted)); 
    mpu6050_data.cursor_lines = MPU6050_TEST_OUTPUT_LINES + MPU6050_TEST_OUTPUT_LINES*MPU6050_SECOND_DEVICE; 

    mpu6050_data.imu1.device_num = DEVICE_ONE; 
    mpu6050_data.imu1.status = MPU6050_OK; 
    mpu6050_data.imu1.st_result = CLEAR; 
    mpu6050_data.imu1.temp_raw = CLEAR; 
    memset((void *)mpu6050_data.imu1.accel_raw, CLEAR, sizeof(mpu6050_data.imu1.accel_raw)); 
    memset((void *)mpu6050_data.imu1.gyro_raw, CLEAR, sizeof(mpu6050_data.imu1.gyro_raw)); 
    mpu6050_data.imu1.temp = CLEAR; 
    memset((void *)mpu6050_data.imu1.accel, CLEAR, sizeof(mpu6050_data.imu1.accel)); 
    memset((void *)mpu6050_data.imu1.gyro, CLEAR, sizeof(mpu6050_data.imu1.gyro)); 

#if MPU6050_SECOND_DEVICE 

    mpu6050_data.imu1.device_num = DEVICE_TWO; 
    mpu6050_data.imu2.status = MPU6050_OK; 
    mpu6050_data.imu2.st_result = CLEAR; 
    mpu6050_data.imu2.temp_raw = CLEAR; 
    memset((void *)mpu6050_data.imu2.accel_raw, CLEAR, sizeof(mpu6050_data.imu2.accel_raw)); 
    memset((void *)mpu6050_data.imu2.gyro_raw, CLEAR, sizeof(mpu6050_data.imu2.gyro_raw)); 
    mpu6050_data.imu2.temp = CLEAR; 
    memset((void *)mpu6050_data.imu2.accel, CLEAR, sizeof(mpu6050_data.imu2.accel)); 
    memset((void *)mpu6050_data.imu2.gyro, CLEAR, sizeof(mpu6050_data.imu2.gyro)); 

#endif   // MPU6050_SECOND_DEVICE 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer - read and record data 
    tim_9_to_11_counter_init(
        mpu6050_data.tim_read, 
        TIM_84MHZ_100US_PSC, 
        MPU6050_TEST_INT_READ_COUNT, 
        TIM_UP_INT_ENABLE); 
    tim_enable(mpu6050_data.tim_read);

    // Periodic (counter update) interrupt timer - display data 
    tim_9_to_11_counter_init(
        mpu6050_data.tim_display, 
        TIM_84MHZ_100US_PSC, 
        MPU6050_TEST_INT_DISPLAY_COUNT, 
        TIM_UP_INT_ENABLE); 
    tim_enable(mpu6050_data.tim_display);

    // UART - serial terminal output 
    uart_init(
        mpu6050_data.uart, 
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
        mpu6050_data.i2c, 
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

    //===================================================
    // MPU-6050 initialization and setup 
    
    // Initialize the device 
    mpu6050_data.imu1.status |= mpu6050_init(
        mpu6050_data.imu1.device_num, 
        mpu6050_data.i2c, 
        MPU6050_ADDR_1,
        standby_mask, 
        MPU6050_DLPF_CFG_1,
        sample_rate_divider,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

#if MPU6050_INT_PIN 

    // Set up the INT pin 
    mpu6050_int_pin_init(GPIOC, PIN_11); 

#endif   // MPU6050_INT_PIN 

    // Run a self-test 
    mpu6050_data.imu1.status |= mpu6050_self_test(mpu6050_data.imu1.device_num, &mpu6050_data.imu1.st_result); 

    // Set the device offsets to calibrate the readings 
    mpu6050_data.imu1.status |= mpu6050_set_offsets(mpu6050_data.imu1.device_num, accel_offsets, gyro_offsets); 

    if (mpu6050_data.imu1.status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

#if MPU6050_SECOND_DEVICE 

    // Initialize the second device 
    mpu6050_data.imu2.status |= mpu6050_init(
        mpu6050_data.imu2.device_num, 
        mpu6050_data.i2c, 
        MPU6050_ADDR_2,
        standby_mask, 
        MPU6050_DLPF_CFG_1,
        sample_rate_divider,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // Run a self-test 
    mpu6050_data.imu2.status |= mpu6050_self_test(mpu6050_data.imu2.device_num, &mpu6050_data.imu2.st_result); 

    // Set the device offsets to calibrate the readings 
    mpu6050_data.imu1.status |= mpu6050_set_offsets(mpu6050_data.imu2.device_num, accel_offsets, gyro_offsets); 

    if (mpu6050_data.imu2.status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

#endif   // MPU6050_SECOND_DEVICE 

    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void mpu6050_test_app()
{
    // Periodically read and record new data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        mpu6050_data.data_count++;

        mpu6050_test_read_record(&mpu6050_data.imu1); 
#if MPU6050_SECOND_DEVICE 
        mpu6050_test_read_record(&mpu6050_data.imu2); 
#endif   // MPU6050_SECOND_DEVICE 
    }

    // Periodically update the displayed data 
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR;

        mpu6050_test_format_output(&mpu6050_data.imu1); 
#if MPU6050_SECOND_DEVICE 
        mpu6050_test_format_output(&mpu6050_data.imu2); 
#endif   // MPU6050_SECOND_DEVICE 

        mpu6050_data.data_count = CLEAR;
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Read and record IMU data 
void mpu6050_test_read_record(mpu6050_test_imu_data_t *imu_data)
{
    // Update the accelerometer, temperature and gyroscope readings 
    imu_data->status = mpu6050_update(imu_data->device_num);

    if (imu_data->status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

    int16_t accel_raw[NUM_AXES], gyro_raw[NUM_AXES];
    float accel[NUM_AXES], gyro[NUM_AXES];

    // Get the raw temperature, accelerometer and gyroscope readings 
    imu_data->temp_raw += (int32_t)mpu6050_get_temp_raw(imu_data->device_num); 
    mpu6050_get_accel_axis(imu_data->device_num, accel_raw); 
    mpu6050_get_gyro_axis(imu_data->device_num, gyro_raw); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    imu_data->temp += mpu6050_get_temp(imu_data->device_num); 
    mpu6050_get_accel_axis_gs(imu_data->device_num, accel); 
    mpu6050_get_gyro_axis_rate(imu_data->device_num, gyro);

    // Record the accel and gyro data 
    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
    {
        imu_data->accel_raw[i] += (int32_t)accel_raw[i];
        imu_data->gyro_raw[i] += (int32_t)gyro_raw[i];
        imu_data->accel[i] += accel[i];
        imu_data->gyro[i] += gyro[i];
    }
}


// Format and output IMU data 
void mpu6050_test_format_output(mpu6050_test_imu_data_t *imu_data)
{
    int16_t temp_raw, accel_raw[NUM_AXES], gyro_raw[NUM_AXES];
    float temp, accel[NUM_AXES], gyro[NUM_AXES];

    // Average the collected data since the last display output and reset the data buffers 
    temp_raw = imu_data->temp_raw / mpu6050_data.data_count;
    temp = imu_data->temp / mpu6050_data.data_count;
    imu_data->temp_raw = CLEAR;
    imu_data->temp = CLEAR;

    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
    {
        accel_raw[i] = imu_data->accel_raw[i] / mpu6050_data.data_count;
        gyro_raw[i] = imu_data->gyro_raw[i] / mpu6050_data.data_count;
        accel[i] = imu_data->accel[i] / mpu6050_data.data_count;
        gyro[i] = imu_data->gyro[i] / mpu6050_data.data_count;

        imu_data->accel_raw[i] = CLEAR;
        imu_data->gyro_raw[i] = CLEAR;
        imu_data->accel[i] = CLEAR;
        imu_data->gyro[i] = CLEAR;
    }

    // Reset the cursor position to overwrite the old data 
    uart_cursor_move(mpu6050_data.uart, UART_CURSOR_UP, mpu6050_data.cursor_lines); 
    uart_send_str(mpu6050_data.uart, "\r"); 

    // Format the raw data into a string 
    snprintf(mpu6050_data.output_raw, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "temp_r = %d   \r\n"
             "ax_r = %d   \r\n"
             "ay_r = %d   \r\n"
             "az_r = %d   \r\n"
             "gx_r = %d   \r\n"
             "gy_r = %d   \r\n"
             "gz_r = %d   \r\n\n", 
             temp_raw, 
             accel_raw[X_AXIS], 
             accel_raw[Y_AXIS], 
             accel_raw[Z_AXIS], 
             gyro_raw[X_AXIS], 
             gyro_raw[Y_AXIS], 
             gyro_raw[Z_AXIS]); 

    // Format the formatted data into a striing 
    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "temp_f = %d   \r\n"
             "ax_f = %d   \r\n"
             "ay_f = %d   \r\n"
             "az_f = %d   \r\n"
             "gx_f = %d   \r\n"
             "gy_f = %d   \r\n"
             "gz_f = %d   \r\n\n", 
             (int16_t)(temp * SCALE_100), 
             (int16_t)(accel[X_AXIS] * SCALE_100), 
             (int16_t)(accel[Y_AXIS] * SCALE_100), 
             (int16_t)(accel[Z_AXIS] * SCALE_100), 
             (int16_t)(gyro[X_AXIS] * SCALE_100), 
             (int16_t)(gyro[Y_AXIS] * SCALE_100), 
             (int16_t)(gyro[Z_AXIS] * SCALE_100)); 

    // Display the data in the serial terminal 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_raw); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted);
}


// Output the driver status and stop program execution 
void mpu6050_test_fault_state(void)
{
    tim_disable(mpu6050_data.tim_read); 
    tim_disable(mpu6050_data.tim_display); 

    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "\r\nMPU-6050 1\r\n"
             "Status: %lu"
             "\r\nSelf-Test: %u", 
             mpu6050_data.imu1.status, 
             mpu6050_data.imu1.st_result); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted); 

#if MPU6050_SECOND_DEVICE 

    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "\r\nMPU-6050 2\r\n"
             "Status: %lu"
             "\r\nSelf-Test: %u", 
             mpu6050_data.imu2.status, 
             mpu6050_data.imu2.st_result); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted); 

#endif   // MPU6050_SECOND_DEVICE 

    while (TRUE); 
}

//=======================================================================================
