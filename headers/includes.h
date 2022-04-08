/**
 * @file includes.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief One place to organize included files for a project.
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _INCLUDES_H_
#define _INCLUDES_H_

//=======================================================================================
// Includes 

// Device drivers 
#include "HW_125_microSD_driver.h"
#include "mpu6050_driver.h"
#include "potentiometer_read.h"
#include "wayintop_lcd_driver.h"

// Communication drivers 
#include "i2c_comm.h"
#include "uart_comm.h"

// Other drivers 
#include "timers.h"

//=======================================================================================

#endif  // _INCLUDES_H_
