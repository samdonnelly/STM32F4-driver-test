/**
 * @file m8q_config.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS configuration file interface 
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

// Number of messages in a configuration packet 
#define M8Q_CONFIG_NUM_MSG_PKT_0 12 

// Max length of a single config message in a packet 
#define M8Q_CONFIG_MAX_MSG_LEN 130 

//=======================================================================================


//=======================================================================================
// Config messages 

extern const char m8q_config_no_pkt[]; 
extern const char m8q_config_pkt_0[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_MSG_LEN]; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief M8Q configuration message copy 
 * 
 * @details This function copies the configuration messages defined in the m8q_config 
 *          file into the array passed to this function. This array can then be passed to 
 *          the m8q_init function to configure the receiver. 
 *          
 *          The messages defined in the config file are meant to configure the settings/
 *          behavior of the receiver. The M8Q doesn't have flash memory so it must be 
 *          reconfigured every time it looses power and battery backup power. These 
 *          messages are carefully constructed according to the message format in the 
 *          devices protocol datasheet. 
 *          
 *          The configuration messages are defined in a separate config file and copied 
 *          into an array so that there is a central location to define the messages 
 *          whether they are used in a project or not. 
 * 
 * @param config_msgs 
 */
void m8q_config_copy(char config_msgs[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_MSG_LEN]); 

//=======================================================================================

#endif  // _M8Q_CONFIG_H_
