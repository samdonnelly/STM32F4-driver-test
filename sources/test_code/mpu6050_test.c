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

// //==================================================
// // Setter and getter wrapper functions 

// void mpu6050_test_get_state(void);       // Get the controller state 
// void mpu6050_test_get_fault(void);       // Get the controller fault code 
// void mpu6050_test_set_reset(void);       // Set the controller reset flag 
// void mpu6050_test_set_low_pwr(void);     // Set the controller low power flag 
// void mpu6050_test_clear_low_pwr(void);   // Clear the controller low power flag 
// void mpu6050_test_get_ax_raw(void);      // Get the raw acceleration value in the x-axis 
// void mpu6050_test_get_ay_raw(void);      // Get the raw acceleration value in the y-axis 
// void mpu6050_test_get_az_raw(void);      // Get the raw acceleration value in the z-axis 
// void mpu6050_test_get_ax(void);          // Get the acceleration in the x-axis 
// void mpu6050_test_get_ay(void);          // Get the acceleration in the y-axis 
// void mpu6050_test_get_az(void);          // Get the acceleration in the z-axis 
// void mpu6050_test_get_gx_raw(void);      // Get the raw angular velocity value in the x-axis 
// void mpu6050_test_get_gy_raw(void);      // Get the raw angular velocity value in the y-axis 
// void mpu6050_test_get_gz_raw(void);      // Get the raw angular velocity value in the z-axis 
// void mpu6050_test_get_gx(void);          // Get the angular velocity in the x-axis 
// void mpu6050_test_get_gy(void);          // Get the angular velocity in the y-axis 
// void mpu6050_test_get_gz(void);          // Get the angular velocity in the z-axis 
// void mpu6050_test_get_temp_raw(void);    // Get the raw temperature value 
// void mpu6050_test_get_temp(void);        // Get the temperature 

// //==================================================


// //==================================================
// // UI functions 

// // Format file name 
// uint8_t mpu6050_test_format_input(
//     char *buff, 
//     uint16_t *data); 


// // Print data 
// void mpu6050_test_print(
//     char *str, 
//     int16_t data); 

// //==================================================

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

// // Data record 
// typedef struct mpu6050_test_record_s 
// {
//     uint16_t read_len;                       // Read data size (bytes) 
//     uint8_t  cmd_index;                      // For indixing function pointers 
//     char cmd_buff[MPU6050_CMD_SIZE];         // Stores user commands 
// } 
// mpu6050_test_record_t; 

// // Data record instance 
// static mpu6050_test_record_t mpu6050_test_record; 


// // Command pointers 
// typedef struct hw125_user_cmds_s 
// {
//     char user_cmds[MPU6050_CMD_SIZE];        // Stores the defined user input commands 
//     void (*mpu6050_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
// }
// hw125_user_cmds_t; 

// // User commands 
// static hw125_user_cmds_t cmd_table[MPU6050_NUM_TEST_CMDS] = 
// {
//     {"state",        &mpu6050_test_get_state}, 
//     {"fault",        &mpu6050_test_get_fault}, 
//     {"reset",        &mpu6050_test_set_reset}, 
//     {"lp_set",       &mpu6050_test_set_low_pwr}, 
//     {"lp_clear",     &mpu6050_test_clear_low_pwr}, 
//     {"accel_x_raw",  &mpu6050_test_get_ax_raw}, 
//     {"accel_y_raw",  &mpu6050_test_get_ay_raw}, 
//     {"accel_z_raw",  &mpu6050_test_get_az_raw}, 
//     {"accel_x",      &mpu6050_test_get_ax}, 
//     {"accel_y",      &mpu6050_test_get_ay}, 
//     {"accel_z",      &mpu6050_test_get_az}, 
//     {"gyro_x_raw",   &mpu6050_test_get_gx_raw}, 
//     {"gyro_y_raw",   &mpu6050_test_get_gy_raw}, 
//     {"gyro_z_raw",   &mpu6050_test_get_gz_raw}, 
//     {"gyro_x",       &mpu6050_test_get_gx}, 
//     {"gyro_y",       &mpu6050_test_get_gy}, 
//     {"gyro_z",       &mpu6050_test_get_gz}, 
//     {"temp_raw",     &mpu6050_test_get_temp_raw}, 
//     {"temp",         &mpu6050_test_get_temp} 
// }; 


// // Controls when to read user input 
// uint8_t mpu6050_action = 0; 

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
    uint8_t mpu6050_init_status = mpu6050_init(I2C1, 
                                               MPU6050_ADDR_1,
                                               0x00, 
                                               DLPF_CFG_1,
                                               SMPLRT_DIV_0,
                                               AFS_SEL_4,
                                               FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    if (!mpu6050_init_status) uart_sendstring(USART2, "Device seen\r\n");
    else uart_sendstring(USART2, "Device not seen\r\n");

    //===================================================

    //===================================================
    // Setup 

#if MPU6050_CONTROLLER_TEST 

    // Set up the INT pin 
    mpu6050_int_pin_init(GPIOC, PIN_11); 

    // Controller init 
    mpu6050_controller_init(); 

    // Initialize the state machine test code 
    state_machine_init(MPU6050_NUM_TEST_CMDS); 

#else   // MPU6050_CONTROLLER_TEST 

    // MPU6050 self-test 
    uint8_t mpu_self_test_result = mpu6050_self_test();
    uart_sendstring(USART2, "MPU6050 Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2); 

    // Calibrate the device 
    mpu6050_calibrate();

#endif   // MPU6050_CONTROLLER_TEST 

    //===================================================
} 


// Test code 
void mpu6050_test_app()
{
#if MPU6050_CONTROLLER_TEST 

    // // Local variables 
    // mpu6050_test_record.cmd_index = 0xFF; 

    // if (mpu6050_action)
    // {
    //     mpu6050_action = CLEAR; 
    //     uart_sendstring(USART2, "\r\n>>> "); 
    // }
    // else
    // {
    //     // Get the info from the user 
    //     if (uart_data_ready(USART2))
    //     {
    //         mpu6050_action = SET; 

    //         // Retrieve and format the input 
    //         uart_getstr(USART2, mpu6050_test_record.cmd_buff, UART_STR_TERM_CARRIAGE); 

    //         // Format the input and check for validity 
    //         if (mpu6050_test_format_input(mpu6050_test_record.cmd_buff, 
    //                                       &mpu6050_test_record.read_len))
    //         {
    //             // Compare the input to the defined user commands 
    //             for (uint8_t i = 0; i < MPU6050_NUM_TEST_CMDS; i++) 
    //             {
    //                 if (str_compare(mpu6050_test_record.cmd_buff, 
    //                                 cmd_table[i].user_cmds, 
    //                                 BYTE_0)) 
    //                 {
    //                     mpu6050_test_record.cmd_index = i; 
    //                     break; 
    //                 }
    //             }

    //             // Use the index to call the function as needed 
    //             if (mpu6050_test_record.cmd_index != 0xFF) 
    //             {
    //                 (cmd_table[mpu6050_test_record.cmd_index].mpu6050_func_ptrs_t)(); 
    //             } 
    //         }
    //     }
    // }

    // mpu6050_controller(); 


    //==================================================
    // Controller test code 

    // Local variables 

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
                        (m8q_state_func[i].setter_1)(); 
                        break; 

                    case MPU6050_GETTER_PTR_1: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_1)()); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_2: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)(m8q_state_func[i].getter_2)()); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_3: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (m8q_state_func[i].getter_3)()); 
                        uart_send_new_line(USART2); 
                        break; 

                    case MPU6050_GETTER_PTR_4: 
                        uart_send_new_line(USART2); 
                        uart_send_integer(
                            USART2, 
                            (int16_t)((m8q_state_func[i].getter_4)() * NO_DECIMAL_SCALAR)); 
                        uart_send_new_line(USART2); 
                        break; 

                    default: 
                        break; 
                }
            }
        }
    }

    // Call the device controller 
    mpu6050_controller(); 

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
    mpu6050_temp_read();
    mpu6050_accel_read(); 
    mpu6050_gyro_read(); 

    // Get the accelerometer and gyroscope data 
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp() * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(mpu6050_get_accel_x() * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(mpu6050_get_accel_y() * NO_DECIMAL_SCALAR);
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(mpu6050_get_accel_z() * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_X_AXIS]  = (int16_t)(mpu6050_get_gyro_x() * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Y_AXIS]  = (int16_t)(mpu6050_get_gyro_y() * NO_DECIMAL_SCALAR);
    mpu6050_gyro[GYRO_Z_AXIS]  = (int16_t)(mpu6050_get_gyro_z() * NO_DECIMAL_SCALAR); 

    //==============================================================
    
    //==============================================================
    // Display the results 

    uart_sendstring(USART2, "temp = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ax = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "ay = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "az = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gx = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gy = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    uart_sendstring(USART2, "gz = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================

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

// // Get the controller state 
// void mpu6050_test_get_state(void)
// {
//     mpu6050_test_print("state: ", (int16_t)mpu6050_get_state()); 
// }


// // Get the controller fault code 
// void mpu6050_test_get_fault(void)
// {
//     mpu6050_test_print("fault code: ", (int16_t)mpu6050_get_fault_code()); 
// }


// // Set the controller reset flag 
// void mpu6050_test_set_reset(void)
// {
//     mpu6050_set_reset_flag(); 
// }


// // Set the controller low power flag 
// void mpu6050_test_set_low_pwr(void)
// {
//     mpu6050_set_low_power(); 
// }


// // Clear the controller low power flag 
// void mpu6050_test_clear_low_pwr(void)
// {
//     mpu6050_clear_low_power(); 
// }


// // Get the raw acceleration value in the x-axis 
// void mpu6050_test_get_ax_raw(void)
// {
//     mpu6050_test_print("ax raw: ", mpu6050_get_accel_x_raw()); 
// }


// // Get the raw acceleration value in the y-axis 
// void mpu6050_test_get_ay_raw(void)
// {
//     mpu6050_test_print("ay raw: ", mpu6050_get_accel_y_raw()); 
// }


// // Get the raw acceleration value in the z-axis 
// void mpu6050_test_get_az_raw(void)
// {
//     mpu6050_test_print("az raw: ", mpu6050_get_accel_z_raw()); 
// }


// // Get the acceleration in the x-axis 
// void mpu6050_test_get_ax(void)
// {
//     mpu6050_test_print("ax: ", (int16_t)mpu6050_get_accel_x() * NO_DECIMAL_SCALAR); 
// }


// // Get the acceleration in the y-axis 
// void mpu6050_test_get_ay(void)
// {
//     mpu6050_test_print("ay: ", (int16_t)mpu6050_get_accel_y() * NO_DECIMAL_SCALAR); 
// }


// // Get the acceleration in the z-axis 
// void mpu6050_test_get_az(void)
// {
//     mpu6050_test_print("az: ", (int16_t)mpu6050_get_accel_z() * NO_DECIMAL_SCALAR); 
// }


// // Get the raw angular velocity value in the x-axis 
// void mpu6050_test_get_gx_raw(void)
// {
//     mpu6050_test_print("gx raw: ", mpu6050_get_gyro_x_raw()); 
// }


// // Get the raw angular velocity value in the y-axis 
// void mpu6050_test_get_gy_raw(void)
// {
//     mpu6050_test_print("gy raw: ", mpu6050_get_gyro_y_raw()); 
// }


// // Get the raw angular velocity value in the z-axis 
// void mpu6050_test_get_gz_raw(void)
// {
//     mpu6050_test_print("gz raw: ", mpu6050_get_gyro_z_raw()); 
// }


// // Get the angular velocity in the x-axis 
// void mpu6050_test_get_gx(void)
// {
//     mpu6050_test_print("gx: ", (int16_t)(mpu6050_get_gyro_x() * NO_DECIMAL_SCALAR)); 
// }


// // Get the angular velocity in the y-axis 
// void mpu6050_test_get_gy(void)
// {
//     mpu6050_test_print("gy: ", (int16_t)(mpu6050_get_gyro_y() * NO_DECIMAL_SCALAR)); 
// }


// // Get the angular velocity in the z-axis 
// void mpu6050_test_get_gz(void)
// {
//     mpu6050_test_print("gz: ", (int16_t)(mpu6050_get_gyro_z() * NO_DECIMAL_SCALAR)); 
// }


// // Get the raw temperature value 
// void mpu6050_test_get_temp_raw(void)
// {
//     mpu6050_test_print("temp raw: ", mpu6050_get_temp_raw()); 
// }


// // Get the temperature 
// void mpu6050_test_get_temp(void)
// {
//     mpu6050_test_print("temp: ", (int16_t)(mpu6050_get_temp() * NO_DECIMAL_SCALAR)); 
// }


// // Format file input 
// uint8_t mpu6050_test_format_input(
//     char *buff, 
//     uint16_t *data)
// {
//     if (buff == NULL) return FALSE; 

//     // Replace carriage return from input with a null character 
//     for (uint8_t i = 0; i < MPU6050_CMD_SIZE; i++)
//     {
//         if (*buff == UART_STR_TERM_CARRIAGE)
//         {
//             *buff = UART_STR_TERM_NULL; 
//             break; 
//         }
//         buff++; 
//     }

//     return TRUE; 
// }


// // Print data 
// void mpu6050_test_print(
//     char *str, 
//     int16_t data)
// {
//     uart_sendstring(USART2, str); 
//     uart_send_integer(USART2, data); 
//     uart_send_new_line(USART2); 
// }

#endif   // MPU6050_CONTROLLER_TEST

//=======================================================================================
