/**
 * @file includes_drivers.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Includes file for the device drivers 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _INCLUDES_DRIVERS_H_
#define _INCLUDES_DRIVERS_H_

//=======================================================================================
// Includes 

// Device drivers 
#include "hw125_driver.h"
#include "mpu6050_driver.h"
#include "hd44780u_driver.h"
#include "hc05_driver.h"
#include "m8q_driver.h"

// Controllers 
#include "hc05_controller.h" 
#include "hd44780u_controller.h"
#include "hw125_controller.h" 
#include "m8q_controller.h" 
#include "mpu6050_controller.h"

// Communication drivers 
#include "i2c_comm.h"
#include "uart_comm.h"
#include "spi_comm.h"

// Other drivers 
#include "timers.h"
#include "gpio_driver.h"
#include "analog_driver.h"
#include "dma_driver.h" 
#include "interrupt_driver.h"
#include "linked_list_driver.h"
#include "data_filters.h"

// Configuration files 
#include "m8q_config.h"

// Tools 
#include "tools.h"

// STM drivers 
#include "fatfs.h"

//=======================================================================================

#endif  // _INCLUDES_DRIVERS_H_
