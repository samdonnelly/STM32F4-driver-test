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
//=======================================================================================


//=======================================================================================
// Macros 

// Number of messages in a configuration packet 
#define M8Q_CONFIG_NUM_MSG_PKT_0 12 

// Max length of a single config message in a packet 
#define M8Q_CONFIG_MAX_LEN_PKT_0 130 

//=======================================================================================


//=======================================================================================
// Config messages 

extern const char m8q_config_no_pkt[]; 
extern const char m8q_config_pkt_0[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_LEN_PKT_0]; 

//=======================================================================================

#endif  // _M8Q_CONFIG_H_
