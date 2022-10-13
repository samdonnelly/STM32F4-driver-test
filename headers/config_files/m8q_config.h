/**
 * @file m8q_config.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS configuration file 
 * 
 * @version 0.1
 * @date 2022-10-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_CONFIG_H_ 
#define _M8Q_CONFIG_H_ 

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define M8Q_CONFIG_MSG_NUM 12        // Number of M8Q configuration messages on startup 
#define M8Q_CONFIG_MSG_MAX_LEN 150   // Maximum length of an M8Q configuration message 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param config_msgs 
 */
void m8q_config_copy(char config_msgs[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN]); 

//=======================================================================================

#endif  // _M8Q_CONFIG_H_