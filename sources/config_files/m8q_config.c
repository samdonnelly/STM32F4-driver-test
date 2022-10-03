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

//=======================================================================================
// Includes 

#include "m8q_config.h"

//=======================================================================================


//=======================================================================================
// Variables 

static char* m8q_config_msgs[M8Q_CONFIG_MSG_NUM] = 
{
    // Disable default NMEA messages 
    "$PUBX,40,GGA,0,0,0,0,0,0*", 
    "$PUBX,40,GLL,0,0,0,0,0,0*", 
    "$PUBX,40,GSA,0,0,0,0,0,0*", 
    "$PUBX,40,GSV,0,0,0,0,0,0*", 
    "$PUBX,40,RMC,0,0,0,0,0,0*", 
    "$PUBX,40,VTG,0,0,0,0,0,0*", 

    // UBX config messages 
    "B5,62,06,01,0800,F0,00,00,00,00,00,00,00*",      // GGA disable
    "B5,62,06,01,0800,F0,01,00,00,00,00,00,00*",      // GLL disable
    "B5,62,06,01,0800,F0,02,00,00,00,00,00,00*",      // GSA disable
    "B5,62,06,01,0800,F0,03,00,00,00,00,00,00*",      // GSV disable
    "B5,62,06,01,0800,F0,04,00,00,00,00,00,00*",      // RMC disable
    "B5,62,06,01,0800,F0,05,00,00,00,00,00,00*",      // VTG disable 
    "B5,62,06,01,0800,F1,00,01,00,00,00,00,00*",      // POSITION enable 
    "B5,62,06,01,0800,F1,04,01,00,00,00,00,00*",      // TIME enable 
    "B5,62,06,09,0C00,00000000,FFFFFFFF,00000000*",   // Save mask 

    // Enable configured NMEA messages 
    "$PUBX,40,POSITION,1,0,0,0,0,0*", 
    "$PUBX,40,TIME,1,0,0,0,0,0*"
};

//=======================================================================================


//=======================================================================================
// Functions 

// M8Q copy config messages 
void m8q_config_copy(char config_msgs[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN])
{
    // Local variables 
    uint8_t msg_num = 0; 
    uint8_t msg_index = 0; 
    char msg_byte; 

    // Loop through all configuration messages 
    while (msg_num < M8Q_CONFIG_MSG_NUM)
    {
        msg_byte = m8q_config_msgs[msg_num][msg_index]; 

        // Copy a message 
        while (msg_byte != AST_CHAR)
        {
            config_msgs[msg_num][msg_index++] = msg_byte; 
            msg_byte = m8q_config_msgs[msg_num][msg_index]; 
        }

        // Terminate the message 
        config_msgs[msg_num][msg_index++] = CR_CHAR; 
        config_msgs[msg_num][msg_index] = NULL_CHAR; 
        
        // Increment to the next message and reset the message index 
        msg_num++; 
        msg_index = 0; 
    }
}

//=======================================================================================
