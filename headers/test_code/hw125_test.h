/**
 * @file hw125_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW125_TEST_H_
#define _HW125_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

// Libraries 
#include "stdio.h"
#include <stdlib.h> 

//=======================================================================================


//=======================================================================================
// Macros 

// Code options 
#define FORMAT_EXFAT 0 
#define HW125_CONTROLLER_TEST 0     // For switching between driver and controller testing 

// File system 
#define BUFF_SIZE 1024 

// User interface 
#define HW125_NUM_DRIVER_CMDS 11    // Number of driver test commands for the user 
#define CMD_SIZE 50                 // Max user command string length 

// Controller testing 
#define HW125_NUM_USER_CMDS 10      // Number of defined user commands for controller test 
#define HW125_MAX_SETTER_ARGS 1     // Maximum arguments of all function pointer below 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 */
typedef enum {
    FORMAT_FILE_STRING, 
    FORMAT_FILE_MODE, 
    FORMAT_FILE_NUM 
} format_user_input_t; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief FATFS function pointer 
 * 
 * @details This is used only when HW125_CONTROLLER_TEST is zero. This function pointer 
 *          is used with the hw125 driver test user interface to call FatFs functions 
 *          onm command. 
 */
typedef void (*fatfs_func_ptrs_t)(void); 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HW125 setup code 
 * 
 */
void hw125_test_init(void); 


/**
 * @brief HW125 test code 
 * 
 */
void hw125_test_app(void); 

//=======================================================================================

#endif  // _HW125_TEST_H_ 
