/**
 * @file mpu6050_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code 
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
// TODO 
// - Troubleshooting lack of UART data received from PuTTy 
//   - Try removing the INT pin read from the controller and have the data read at an 
//     interval (start at a slow rate to see if it fixes anything). If this works then 
//     maybe have the controller init take a parameter that specified the time between 
//     reads and use a non-blocking timer like the M8Q controller. 
//   - Check the UART ORE bit to see if an overrun error is occuring. 
//   - Try changing the UART baud rate to see if it helps. 
//   - Set UART receive up with DMA so data gets receieved and stored in the background 
//     without needing to wait for the code to loop back and read it. 
// 
// - Add the ability to specify the data read rate in a device controller. For example 
//   at a certain time interval or every 3rd read attempt etc. 
//=======================================================================================


//=======================================================================================
// Notes 
// - UART data received from PuTTy is often being lost during controller testing. This has 
//   happened with the two most recent controller tests (MPU6050 and HW125) and it is 
//   suspected to be due to too much communication happening in the drivers. This 
//   communication is suspected to be taking too much time so data being sent from 
//   PuTTy is being overwritten in the shift register before it can be read from the data 
//   register. 
//   - Changing the MPU6050 "run" state to read all data at once as opposed to reading 
//     temp, accel and gyro sequentially but in separate read operations helped in having 
//     the PuTTy input captured more often but it was still losing data. 
//   - Removing reads entirely from the "run" state make the UART interface work without 
//     an data loss. 
//   - UPDATE: sampling the data every 0.25s made the UART UI function normally. Different 
//             sampling periods have not been tested yet. 
//=======================================================================================


//=======================================================================================
// Function prototypes 

#if MPU6050_CONTROLLER_TEST 

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================


//=======================================================================================
// Global variables 

#if MPU6050_CONTROLLER_TEST 

// User command table 
static state_request_t mpu6050_state_cmds[MPU6050_NUM_TEST_CMDS] = 
{
    {"reset",       0, MPU6050_SETTER_PTR_1, 0}, 
    {"lp_set",      0, MPU6050_SETTER_PTR_1, 0}, 
    {"lp_clear",    0, MPU6050_SETTER_PTR_1, 0}, 
    {"state",       0, MPU6050_GETTER_PTR_1, 0}, 
    {"fault",       0, MPU6050_GETTER_PTR_2, 0}, 
    {"raw_accel_x", 0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_accel_y", 0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_accel_z", 0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_gyro_x",  0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_gyro_y",  0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_gyro_z",  0, MPU6050_GETTER_PTR_3, 0}, 
    {"raw_temp",    0, MPU6050_GETTER_PTR_3, 0}, 
    {"accel_x",     0, MPU6050_GETTER_PTR_4, 0}, 
    {"accel_y",     0, MPU6050_GETTER_PTR_4, 0}, 
    {"accel_z",     0, MPU6050_GETTER_PTR_4, 0}, 
    {"gyro_x",      0, MPU6050_GETTER_PTR_4, 0}, 
    {"gyro_y",      0, MPU6050_GETTER_PTR_4, 0}, 
    {"gyro_z",      0, MPU6050_GETTER_PTR_4, 0}, 
    {"temp",        0, MPU6050_GETTER_PTR_4, 0}, 
    {"execute", 0, 0, 0} 
}; 


// Function pointer table 
static mpu6050_func_ptrs_t m8q_state_func[MPU6050_NUM_TEST_CMDS] = 
{
    {&mpu6050_set_reset_flag, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_low_power, NULL, NULL, NULL, NULL}, 
    {&mpu6050_clear_low_power, NULL, NULL, NULL, NULL}, 
    {NULL, &mpu6050_get_state, NULL, NULL, NULL}, 
    {NULL, NULL, &mpu6050_get_fault_code, NULL, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_accel_x_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_accel_y_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_accel_z_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_gyro_x_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_gyro_y_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_gyro_z_raw, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_temp_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_x}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_y}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_z}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_x}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_y}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_z}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_temp}, 
    {NULL, NULL, NULL, NULL, NULL} 
}; 

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================


//=======================================================================================
// Test code 

// Setup code
void mpu6050_test_init()
{
    //===================================================
    // Peripherals 

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
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // Initialize I2C1
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    //===================================================

    //===================================================
    // Accelerometer initialization 
    
    // Initialize the accelerometer 
    uint8_t mpu6050_init_status = mpu6050_init(DEVICE_ONE, 
                                               I2C1, 
                                               MPU6050_ADDR_1,
                                               0x00, 
                                               DLPF_CFG_1,
                                               SMPLRT_DIV_0,
                                               AFS_SEL_4,
                                               FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    if (!mpu6050_init_status) uart_sendstring(USART2, "Device seen\r\n");
    else uart_sendstring(USART2, "Device not seen\r\n");

#if MPU6050_SECOND_DEVICE 

    // Initialize the second accelerometer 
    mpu6050_init_status = mpu6050_init(DEVICE_TWO, 
                                       I2C1, 
                                       MPU6050_ADDR_2,
                                       0x00, 
                                       DLPF_CFG_1,
                                       SMPLRT_DIV_0,
                                       AFS_SEL_4,
                                       FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    if (!mpu6050_init_status) uart_sendstring(USART2, "Second device seen\r\n");
    else uart_sendstring(USART2, "Second device not seen\r\n");

#endif   // MPU6050_SECOND_DEVICE 

    //===================================================

    //===================================================
    // Setup 

#if MPU6050_CONTROLLER_TEST 

    // // Set up the INT pin 
    // mpu6050_int_pin_init(GPIOC, PIN_11); 

    // Controller init 
    mpu6050_controller_init(
        DEVICE_ONE, 
        TIM9, 
        250000); // 0.25s 

#if MPU6050_SECOND_DEVICE 

    // Controller init 
    mpu6050_controller_init(
        DEVICE_TWO, 
        TIM9, 
        250000); // 0.25s 

#endif   // MPU6050_SECOND_DEVICE 

    // Initialize the state machine test code 
    state_machine_init(MPU6050_NUM_TEST_CMDS); 

#else   // MPU6050_CONTROLLER_TEST 

    // MPU6050 self-test 
    uint8_t mpu_self_test_result = mpu6050_self_test(DEVICE_ONE);
    uart_sendstring(USART2, "MPU6050 Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Provide time for the device to update data so self-test data is not used elsewhere 
    tim_delay_ms(TIM9, 10); 

    // Calibrate the device 
    mpu6050_calibrate(DEVICE_ONE); 

#if MPU6050_SECOND_DEVICE 

    // MPU6050 self-test - second device 
    mpu_self_test_result = mpu6050_self_test(DEVICE_TWO);
    uart_sendstring(USART2, "MPU6050 Second Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Provide time for the device to update data so self-test data is not used elsewhere 
    tim_delay_ms(TIM9, 10); 

    // Calibrate the device 
    mpu6050_calibrate(DEVICE_TWO); 

#endif   // MPU6050_SECOND_DEVICE 

#endif   // MPU6050_CONTROLLER_TEST 

    //===================================================
} 


// Test code 
void mpu6050_test_app()
{
#if MPU6050_CONTROLLER_TEST 

    //==================================================
    // Controller test code 

    // General purpose arguments array 
    static char user_args[1][1]; 

    // Control flags 
    uint8_t arg_convert = 0; 
    uint32_t set_get_status = 0; 
    uint8_t cmd_index = 0; 

    // Determine what to do from user input 
    state_machine_test(
        mpu6050_state_cmds, 
        user_args[0], 
        &cmd_index, 
        &arg_convert, 
        &set_get_status); 

    // Check if there are any setters or getters requested 
    if (set_get_status)
    {
        for (uint8_t i = 0; i < (MPU6050_NUM_TEST_CMDS-1); i++)
        {
            if ((set_get_status >> i) & SET_BIT)
            {
                switch (mpu6050_state_cmds[i].func_ptr_index)
                {
                    case MPU6050_SETTER_PTR_1: 
                        (m8q_state_func[i].setter_1)(DEVICE_ONE); 
                        break; 

                    case MPU6050_GETTER_PTR_1: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_1)(DEVICE_ONE)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_2: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_2)(DEVICE_ONE)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_3: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (m8q_state_func[i].getter_3)(DEVICE_ONE)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_4: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)((m8q_state_func[i].getter_4)(DEVICE_ONE) * NO_DECIMAL_SCALAR)); 
                        uart_send_new_line(USART2); 
                        break; 

                    default: 
                        break; 
                }
            }
        }
    }

    // Call the device controller 
    mpu6050_controller(DEVICE_ONE); 

    //==================================================

#else   // MPU6050_CONTROLLER_TEST

    //==============================================================
    // Control Code 
    //  1. Read raw temperature feedback, format it into degC, print it to serial 
    //  2. Read raw acceleromater feedback, format it into g's, print it to serial 
    //  3. Read raw gyroscope feedback, format it into deg/s, print it to serial 
    //  4. Start a new serial terminal line 
    //  5. Delay 
    //  6. Repeat 
    //==============================================================

    // Local variables 
    static int16_t mpu6050_temp_sensor; 
    static int16_t mpu6050_accel[MPU6050_NUM_AXIS]; 
    static int16_t mpu6050_gyro[MPU6050_NUM_AXIS]; 

    //==============================================================
    // Device data 

    // Update the accelerometer and gyroscope readings 
    mpu6050_temp_read(DEVICE_ONE);
    mpu6050_accel_read(DEVICE_ONE); 
    mpu6050_gyro_read(DEVICE_ONE); 

    // Get the accelerometer and gyroscope data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(mpu6050_get_accel_x(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(mpu6050_get_accel_y(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(mpu6050_get_accel_z(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_X_AXIS]  = (int16_t)(mpu6050_get_gyro_x(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Y_AXIS]  = (int16_t)(mpu6050_get_gyro_y(DEVICE_ONE) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Z_AXIS]  = (int16_t)(mpu6050_get_gyro_z(DEVICE_ONE) * NO_DECIMAL_SCALAR); 

    //==============================================================
    
    //==============================================================
    // Display the results 

    uart_sendstring(USART2, "temp1 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ax1 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ay1 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "az1 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gx1 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gy1 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gz1 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================

#if MPU6050_SECOND_DEVICE 

    // Display results on a new line 
    uart_send_new_line(USART2);

    // Update the accelerometer and gyroscope readings 
    mpu6050_temp_read(DEVICE_TWO);
    mpu6050_accel_read(DEVICE_TWO); 
    mpu6050_gyro_read(DEVICE_TWO); 

    // Get the accelerometer and gyroscope data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(mpu6050_get_accel_x(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(mpu6050_get_accel_y(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(mpu6050_get_accel_z(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_X_AXIS]  = (int16_t)(mpu6050_get_gyro_x(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Y_AXIS]  = (int16_t)(mpu6050_get_gyro_y(DEVICE_TWO) * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Z_AXIS]  = (int16_t)(mpu6050_get_gyro_z(DEVICE_TWO) * NO_DECIMAL_SCALAR); 

    // Display results 
    uart_sendstring(USART2, "temp2 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ax2 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ay2 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "az2 = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gx2 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gy2 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gz2 = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Go up a terminal line 
    uart_sendstring(USART2, "\033[1A"); 

#endif   // MPU6050_SECOND_DEVICE 

    // Delay 
    tim_delay_ms(TIM9, LOOP_DELAY);

    // Go to a new line in the serial terminal 
    // uart_send_new_line(USART2); 
    uart_sendstring(USART2, "\r"); 

#endif   // MPU6050_CONTROLLER_TEST 
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if MPU6050_CONTROLLER_TEST 

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================
