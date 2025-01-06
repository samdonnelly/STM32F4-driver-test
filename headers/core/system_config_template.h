/**
 * @file system_config_template.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief System configuration template 
 * 
 * @details This file is a template and is not to be altered unless there is a system 
 *          configuration setting being added or removed. This means these settings should 
 *          not be used to configure your system. Instead, create a "system_config.h" file 
 *          in the same folder as this file and redefine each of the macros below (i.e. 
 *          copy the contents of this file) and change those to configure your system. 
 *          For example, if you're using FreeRTOS, then change FREERTOS_ENABLE to 1 in 
 *          "system_config.h". "system_config.h" is not tracked so it's specific to your 
 *          use case, but it's included in the build path. This file is tracked but 
 *          excluded from the build path. 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _SYSTEM_CONFIG_TEMPLATE_H_ 
#define _SYSTEM_CONFIG_TEMPLATE_H_ 

//=======================================================================================
// System configuration 

// This needs to be set if using FreeRTOS as it includes additional files and alters 
// some core code that's used with FreeRTOS. Note that the 'RTOS_ENABLE' variable in 
// CMakeLists must be updated to match this macro. 
#define FREERTOS_ENABLE 0 

// This includes interrupt handler definitions that override defaults. Overrides will 
// be used for specific tests. If interrupts are used for a test but the test has no 
// specific override then this should be cleared. 
#define INTERRUPT_OVERRIDE 0 

// Board selection 
// The below list allows for choosing which STM32F4 board to use. Redefine each of the 
// following in the "system_config.h" file and change your selected board to 1. 
#define STM32F4_05xx 0 
#define STM32F4_15xx 0 
#define STM32F4_07xx 0 
#define STM32F4_17xx 0 
#define STM32F4_27xx 0 
#define STM32F4_37xx 0 
#define STM32F4_29xx 0 
#define STM32F4_39xx 0 
#define STM32F4_01xC 0 
#define STM32F4_01xE 0 
#define STM32F4_10Tx 0 
#define STM32F4_10Cx 0 
#define STM32F4_10Rx 0 
#define STM32F4_11xE 0 
#define STM32F4_46xx 0 
#define STM32F4_69xx 0 
#define STM32F4_79xx 0 
#define STM32F4_12Cx 0 
#define STM32F4_12Zx 0 
#define STM32F4_12Rx 0 
#define STM32F4_12Vx 0 
#define STM32F4_13xx 0 
#define STM32F4_23xx 0 

//=======================================================================================


//=======================================================================================
// Application test modes 

//==================================================
// FreeRTOS test 

#define PERIODIC_BLINK_TEST 0        // Highest priority test 
#define MANUAL_BLINK_TEST 0 
#define TASK_SCHEDULING_TEST 0 
#define MEMORY_MANAGEMENT_TEST 0 
#define QUEUE_TEST 0 
#define MUTEX_TEST 0 
#define SEMAPHORE_TEST 0 
#define SOFTWARE_TIMER_TEST_0 0 
#define SOFTWARE_TIMER_TEST_1 0 
#define HARDWARE_INTERRUPT_TEST 0 
#define DEADLOCK_STARVATION_TEST 0 
#define PRIORITY_INVERSION_TEST 0    // Lowest priority test 

//==================================================

//==================================================
// RC Test 

// Devices 
#define RC_SYSTEM_1 0 
#define RC_SYSTEM_2 0 

// Test code 
#define RC_SD_CARD_TEST 0 
#define RC_MOTOR_TEST 0 
#define RC_GROUND_STATION_TEST 0 

//==================================================

//=======================================================================================


//=======================================================================================
// Device test modes 

// Toggle the mode for each device/peripheral/tool test 

//==================================================
// General 

#define HD44780U_ON_I2C_BUS 0             // Turn HD44780U screen off if on the same I2C 
                                          // bus as another device and not being used. 

//==================================================

//==================================================
// HC-05 

#define HC05_CONTROLLER_TEST 0            // For switching between driver and controller testing 

//==================================================

//==================================================
// HD44780U 

#define HD44780U_CONTROLLER_TEST 0        // For switching between driver and controller testing 
#define HD44780U_BACKLIGHT_TEST 0         // Backlight control test 
#define HD44780U_DISPLAY_TEST 0           // Display on/off test 
#define HD44780U_CURSOR_TEST 0            // Cursor visibility test 
#define HD44780U_BLINK_TEST 0             // Cursor blink test 

//==================================================

//==================================================
// HW125 (SD card) 

#define FORMAT_EXFAT 0 
#define HW125_CONTROLLER_TEST 0           // For switching between driver and controller testing 

//==================================================

//==================================================
// LSM303AGR 

// Only one mode is meant to be used at a time. The first macro under each mode is the 
// main/parent mode setter. Subsequent macros under the same mode are only used once the 
// mode is active. 

// Magnetometer axis data mode 
#define LSM303AGR_TEST_AXIS 0             // Magnetometer axis data read 

// Magnetometer heading mode 
#define LSM303AGR_TEST_HEADING 0          // Magnetometer heading read (compass) 
#define LSM303AGR_TEST_CALIBRATION 0      // Magnetometer heading calibration 

//==================================================

//==================================================
// M8Q 
//==================================================

//==================================================
// MPU-6050 driver test 

// Test control 
#define MPU6050_CONTROLLER_TEST 0        // Switch between driver and controller testing 
#define MPU6050_SECOND_DEVICE 0          // Include the test code for a second device 
#define MPU6050_INT_PIN 0                // Interrupt pin enable 

//==================================================

//==================================================
// nRF24L01 driver test 

// Device 
#define NRF24L01_SYSTEM_1 0              // Enable device 1 code 
#define NRF24L01_SYSTEM_2 0              // Enable device 2 code 

// Test code 
#define NRF24L01_HEARTBEAT 0             // Heartbeat 
#define NRF24L01_MANUAL_CONTROL 0        // Perform actions based on user input 

//==================================================

//=======================================================================================

#endif   // _SYSTEM_CONFIG_TEMPLATE_H_ 
