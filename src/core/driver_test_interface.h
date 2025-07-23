/**
 * @file driver_test_interface.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver test initialization and application code interface 
 * 
 * @version 0.1
 * @date 2024-03-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _DRIVER_TEST_INTERFACE_H_ 
#define _DRIVER_TEST_INTERFACE_H_ 

//=======================================================================================
// Includes 

#include "includes_app.h"
#include "includes_cpp_app.h"

//=======================================================================================


//=======================================================================================
// Driver test interface 

/**
 * @brief Driver test initialization code - called once 
 */
void DriverTestInit(void); 


/**
 * @brief Driver test application code - called repeatedly 
 */
void DriverTestApp(void); 

//=======================================================================================

#endif   // _DRIVER_TEST_INTERFACE_H_ 
