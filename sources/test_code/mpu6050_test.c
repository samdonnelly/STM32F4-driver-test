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
    {"dev_one",     0, MPU6050_SETTER_PTR_2, 0},   // 0
    {"dev_two",     0, MPU6050_SETTER_PTR_2, 0},   // 1
    {"reset",       0, MPU6050_SETTER_PTR_1, 0},   // 2
    {"lp_set",      0, MPU6050_SETTER_PTR_1, 0},   // 3
    {"lp_clear",    0, MPU6050_SETTER_PTR_1, 0},   // 4
    {"state",       0, MPU6050_GETTER_PTR_1, 0},   // 5
    {"fault",       0, MPU6050_GETTER_PTR_2, 0},   // 6
    {"raw_accel_x", 0, MPU6050_GETTER_PTR_3, 0},   // 7
    {"raw_accel_y", 0, MPU6050_GETTER_PTR_3, 0},   // 8
    {"raw_accel_z", 0, MPU6050_GETTER_PTR_3, 0},   // 9
    {"raw_gyro_x",  0, MPU6050_GETTER_PTR_3, 0},   // 10
    {"raw_gyro_y",  0, MPU6050_GETTER_PTR_3, 0},   // 11
    {"raw_gyro_z",  0, MPU6050_GETTER_PTR_3, 0},   // 12
    {"raw_temp",    0, MPU6050_GETTER_PTR_3, 0},   // 13
    {"accel_x",     0, MPU6050_GETTER_PTR_4, 0},   // 14
    {"accel_y",     0, MPU6050_GETTER_PTR_4, 0},   // 15
    {"accel_z",     0, MPU6050_GETTER_PTR_4, 0},   // 16
    {"gyro_x",      0, MPU6050_GETTER_PTR_4, 0},   // 17
    {"gyro_y",      0, MPU6050_GETTER_PTR_4, 0},   // 18
    {"gyro_z",      0, MPU6050_GETTER_PTR_4, 0},   // 19
    {"temp",        0, MPU6050_GETTER_PTR_4, 0},   // 20
    {"execute", 0, 0, 0}                           // 21
}; 


// Function pointer table 
static mpu6050_func_ptrs_t m8q_state_func[MPU6050_NUM_TEST_CMDS] = 
{
    {NULL, &mpu6050_cntrl_test_device_one, NULL, NULL, NULL, NULL}, 
    {NULL, &mpu6050_cntrl_test_device_two, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_reset_flag, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_set_low_power, NULL, NULL, NULL, NULL, NULL}, 
    {&mpu6050_clear_low_power, NULL, NULL, NULL, NULL, NULL}, 
    {NULL, NULL, &mpu6050_get_state, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, &mpu6050_get_fault_code, NULL, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_x_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_y_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_accel_z_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_x_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_y_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_gyro_z_raw, NULL}, 
    {NULL, NULL, NULL, NULL, &mpu6050_get_temp_raw, NULL}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_accel_x}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_accel_y}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_accel_z}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_gyro_x}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_gyro_y}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_gyro_z}, 
    {NULL, NULL, NULL, NULL, NULL, &mpu6050_get_temp}, 
    {NULL, NULL, NULL, NULL, NULL, NULL} 
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

    // Set the device number 
    mpu6050_cntrl_test_device = DEVICE_ONE; 

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
                        (m8q_state_func[i].setter_1)(mpu6050_cntrl_test_device); 
                        break; 

                    case MPU6050_SETTER_PTR_2: 
                        (m8q_state_func[i].setter_2)(); 
                        break; 

                    case MPU6050_GETTER_PTR_1: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_1)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_2: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_2)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_3: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (m8q_state_func[i].getter_3)(mpu6050_cntrl_test_device)); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_4: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)((m8q_state_func[i].getter_4)(
                                mpu6050_cntrl_test_device) * NO_DECIMAL_SCALAR)); 
                        uart_send_new_line(USART2); 
                        break; 

                    default: 
                        break; 
                }
            }
        }
    }

    // Call the device controller 
    mpu6050_controller(mpu6050_cntrl_test_device); 

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
