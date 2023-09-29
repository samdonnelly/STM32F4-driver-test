/**
 * @file mpu6050_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU6050 test code 
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
// Function prototypes 

#if MPU6050_CONTROLLER_TEST 

void mpu6050_cntrl_test_device_one(void);   // Choose device one 
void mpu6050_cntrl_test_device_two(void);   // Choose device two 

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================


//=======================================================================================
// Global variables 

#if MPU6050_CONTROLLER_TEST 

// User command table 
static state_request_t mpu6050_state_cmds[MPU6050_NUM_TEST_CMDS] = 
{
    {"dev_one",     SMT_ARGS_0, SMT_STATE_FUNC_PTR_3, SMT_ARG_BUFF_POS_0},   // 0
    {"dev_two",     SMT_ARGS_0, SMT_STATE_FUNC_PTR_3, SMT_ARG_BUFF_POS_0},   // 1
    {"lp_set",      SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0},   // 2
    {"lp_clear",    SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0},   // 3
    {"sample",      SMT_ARGS_1, SMT_STATE_FUNC_PTR_2, SMT_ARG_BUFF_POS_0},   // 4
    {"read_state",  SMT_ARGS_1, SMT_STATE_FUNC_PTR_2, SMT_ARG_BUFF_POS_1},   // 5
    {"read",        SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0},   // 6
    {"reset",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_1, SMT_ARG_BUFF_POS_0},   // 7
    {"state",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_4, SMT_ARG_BUFF_POS_0},   // 8
    {"fault",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_5, SMT_ARG_BUFF_POS_0},   // 9
    {"accel_raw",   SMT_ARGS_0, SMT_STATE_FUNC_PTR_6, SMT_ARG_BUFF_POS_0},   // 10
    {"gyro_raw",    SMT_ARGS_0, SMT_STATE_FUNC_PTR_6, SMT_ARG_BUFF_POS_0},   // 11
    {"temp_raw",    SMT_ARGS_0, SMT_STATE_FUNC_PTR_7, SMT_ARG_BUFF_POS_0},   // 12
    {"accel",       SMT_ARGS_0, SMT_STATE_FUNC_PTR_8, SMT_ARG_BUFF_POS_0},   // 13
    {"gyro",        SMT_ARGS_0, SMT_STATE_FUNC_PTR_8, SMT_ARG_BUFF_POS_0},   // 14
    {"temp",        SMT_ARGS_0, SMT_STATE_FUNC_PTR_9, SMT_ARG_BUFF_POS_0},   // 15
    {"execute", 0, 0, 0}                                                     // 16
}; 


// Function pointer table 
static mpu6050_func_ptrs_t m8q_state_func[MPU6050_NUM_TEST_CMDS] = 
{
    {NULL, NULL, &mpu6050_cntrl_test_device_one, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, &mpu6050_cntrl_test_device_two, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_low_power, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_clear_low_power, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, &mpu6050_set_smpl_type, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, &mpu6050_set_read_state, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_read_flag, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_reset_flag, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_state, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_fault_code, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_accel_raw, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_gyro_raw, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, NULL, &mpu6050_get_temp_raw, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, &mpu6050_get_accel, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, &mpu6050_get_gyro, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &mpu6050_get_temp}, 
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} 
}; 


// Device selector 
static device_number_t mpu6050_cntrl_test_device; 

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================


//=======================================================================================
// Test code 

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
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

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

#if MPU6050_LCD_ON_BUS 

    //===================================================
    // HD44780U LCD setup 

    // Must come before setup of other devices on the same I2C bus 

    // Driver 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH);

    // Contoller 
    hd44780u_controller_init(TIM9); 

    //===================================================

#endif   // MPU6050_LCD_ON_BUS 

    //===================================================
    // Accelerometer initialization 

    // NOTE: If the HD44780U LCD screen is connected to the same I2C bus then that needs to be 
    //       initialized first. For some reason the uninitialized screen interferes with the bus 
    //       and prevents the mpu6050 from being initialized properly. See above. 
    
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
    if (!mpu6050_get_status(DEVICE_ONE)) uart_sendstring(USART2, "Device seen\r\n");
    else uart_sendstring(USART2, "Device not seen\r\n");

    
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
    if (!mpu6050_get_status(DEVICE_TWO)) uart_sendstring(USART2, "Second device seen\r\n");
    else uart_sendstring(USART2, "Second device not seen\r\n");

    #endif   // MPU6050_SECOND_DEVICE 

    //===================================================

    //===================================================
    // Setup 

#if MPU6050_CONTROLLER_TEST 

    // Set the device number 
    mpu6050_cntrl_test_device = DEVICE_ONE; 

    // Controller init 
    mpu6050_controller_init(
        DEVICE_ONE, 
        TIM9, 
        MPU6050_DEV1_RATE); 

    
    #if MPU6050_SECOND_DEVICE 

    // Controller init 
    mpu6050_controller_init(
        DEVICE_TWO, 
        TIM9, 
        MPU6050_DEV2_RATE); 

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
    uart_sendstring(USART2, "MPU6050 Second Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Provide time for the device to update data so self-test data is not used elsewhere 
    tim_delay_ms(TIM9, MPU6050_DRIVER_ST_DELAY); 

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
    static char user_args[STATE_USER_TEST_INPUT]; 

    // Arguments for mpu6050_setter_ptr_2 
    static uint8_t param[2]; 

    // Sensor buffers 
    static float imu_data_float[MPU6050_NUM_AXIS]; 
    static int16_t imu_data_int[MPU6050_NUM_AXIS]; 

    // Control flags 
    uint8_t arg_convert = 0; 
    uint32_t set_get_status = 0; 
    uint8_t cmd_index = 0; 

    // Determine what to do from user input 
    state_machine_test(
        mpu6050_state_cmds, 
        user_args, 
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
                    case SMT_STATE_FUNC_PTR_1: 
                        (m8q_state_func[i].setter_1)(
                            mpu6050_cntrl_test_device); 
                        break; 

                    case SMT_STATE_FUNC_PTR_2: 
                        (m8q_state_func[i].setter_2)(
                            mpu6050_cntrl_test_device, 
                            param[mpu6050_state_cmds[i].arg_buff_index]); 
                        break; 

                    case SMT_STATE_FUNC_PTR_3: 
                        (m8q_state_func[i].setter_3)(); 
                        break; 

                    case SMT_STATE_FUNC_PTR_4: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_1)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_5: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_2)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_6: 
                        (m8q_state_func[i].getter_3)(
                            mpu6050_cntrl_test_device, 
                            &imu_data_int[MPU6050_X_AXIS], 
                            &imu_data_int[MPU6050_Y_AXIS], 
                            &imu_data_int[MPU6050_Z_AXIS]); 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, "X: ");
                        uart_send_integer(USART2, imu_data_int[MPU6050_X_AXIS]); 
                        uart_sendstring(USART2, "  Y: ");
                        uart_send_integer(USART2, imu_data_int[MPU6050_Y_AXIS]); 
                        uart_sendstring(USART2, "  Z: ");
                        uart_send_integer(USART2, imu_data_int[MPU6050_Z_AXIS]); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_7: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (m8q_state_func[i].getter_4)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_8: 
                        (m8q_state_func[i].getter_5)(
                            mpu6050_cntrl_test_device, 
                            &imu_data_float[MPU6050_X_AXIS], 
                            &imu_data_float[MPU6050_Y_AXIS], 
                            &imu_data_float[MPU6050_Z_AXIS]); 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, "X: ");
                        uart_send_integer(USART2, 
                            (int16_t)(imu_data_float[MPU6050_X_AXIS] * SCALE_100)); 
                        uart_sendstring(USART2, "  Y: ");
                        uart_send_integer(USART2, 
                            (int16_t)(imu_data_float[MPU6050_Y_AXIS] * SCALE_100)); 
                        uart_sendstring(USART2, "  Z: ");
                        uart_send_integer(USART2, 
                            (int16_t)(imu_data_float[MPU6050_Z_AXIS] * SCALE_100)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case SMT_STATE_FUNC_PTR_9: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)((m8q_state_func[i].getter_6)(
                                mpu6050_cntrl_test_device) * SCALE_100)); 
                        uart_send_new_line(USART2); 
                        break; 

                    default: 
                        break; 
                }
            }
        }
    }

    // Check if user argument input should be converted and assigned 
    if (arg_convert)
    {
        switch (mpu6050_state_cmds[cmd_index].func_ptr_index)
        {
            case SMT_STATE_FUNC_PTR_2: 
                param[mpu6050_state_cmds[cmd_index].arg_buff_index] = atoi(user_args); 
                break; 

            default: 
                break; 
        }
    }

    // Call the device controller 
    mpu6050_controller(mpu6050_cntrl_test_device); 

    //==================================================

#else   // MPU6050_CONTROLLER_TEST

    //==================================================
    // Driver test code 

    // Local variables 
    static int16_t mpu6050_temp_sensor; 
    static float mpu6050_accel[MPU6050_NUM_AXIS]; 
    static float mpu6050_gyro[MPU6050_NUM_AXIS]; 

    // Update the accelerometer, temperature and gyroscope readings for device one 
    mpu6050_read_all(DEVICE_ONE); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_ONE) * SCALE_100); 
    mpu6050_get_accel(
        DEVICE_ONE, 
        &mpu6050_accel[MPU6050_X_AXIS], 
        &mpu6050_accel[MPU6050_Y_AXIS], 
        &mpu6050_accel[MPU6050_Z_AXIS]); 
    mpu6050_get_gyro(
        DEVICE_ONE, 
        &mpu6050_gyro[MPU6050_X_AXIS], 
        &mpu6050_gyro[MPU6050_Y_AXIS], 
        &mpu6050_gyro[MPU6050_Z_AXIS]); 

    // Display the first device results - values are scaled to remove decimal 
    uart_sendstring(USART2, "temp1 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "ax1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "ay1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "az1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gx1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gy1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gz1 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);


#if MPU6050_SECOND_DEVICE 

    // Display second device results on a new line 
    uart_send_new_line(USART2);

    // Update the accelerometer, temperature and gyroscope readings for device two 
    mpu6050_read_all(DEVICE_TWO); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(DEVICE_TWO) * SCALE_100); 
    mpu6050_get_accel(
        DEVICE_TWO, 
        &mpu6050_accel[MPU6050_X_AXIS], 
        &mpu6050_accel[MPU6050_Y_AXIS], 
        &mpu6050_accel[MPU6050_Z_AXIS]); 
    mpu6050_get_gyro(
        DEVICE_TWO, 
        &mpu6050_gyro[MPU6050_X_AXIS], 
        &mpu6050_gyro[MPU6050_Y_AXIS], 
        &mpu6050_gyro[MPU6050_Z_AXIS]); 

    // Display the second device results 
    uart_sendstring(USART2, "temp2 = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "ax2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "ay2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "az2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_accel[MPU6050_Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gx2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_X_AXIS] * SCALE_100)); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gy2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_Y_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "gz2 = ");
    uart_send_integer(USART2, (int16_t)(mpu6050_gyro[MPU6050_Z_AXIS] * SCALE_100));
    uart_send_spaces(USART2, UART_SPACE_2);

    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 

#endif   // MPU6050_SECOND_DEVICE 

    // Delay 
    tim_delay_ms(TIM9, MPU6050_DRIVER_LOOP_DELAY);

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 

    //==================================================

#endif   // MPU6050_CONTROLLER_TEST 
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if MPU6050_CONTROLLER_TEST 

// Choose device one 
void mpu6050_cntrl_test_device_one(void)
{
    mpu6050_cntrl_test_device = DEVICE_ONE; 
}


// Choose device two 
void mpu6050_cntrl_test_device_two(void)
{
    mpu6050_cntrl_test_device = DEVICE_TWO; 
}

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================
