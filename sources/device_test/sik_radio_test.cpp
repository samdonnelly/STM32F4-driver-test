/**
 * @file sik_radio_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK radio driver test 
 * 
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "sik_radio_test.h" 

extern "C"
{
    #include "standard/mavlink.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define SIK_TEST_MSG_BUFF_SIZE 200 

//=======================================================================================


//=======================================================================================
// Global data 

// Mavlink data 
typedef struct mavlink_data_s 
{
    int channel; 
    mavlink_message_t msg; 
    char msg_buff[SIK_TEST_MSG_BUFF_SIZE];   // Should this be a circular buffer 
    uint16_t msg_buff_index; 
    mavlink_status_t status; 
}
mavlink_data_t; 

static mavlink_data_t mavlink_data; 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    mavlink_data.channel = MAVLINK_COMM_0; 
    memset((void *)mavlink_data.msg_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    mavlink_data.msg_buff_index = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Test code 

void sik_radio_test_app(void)
{
    // Read data somehow. Could be polling, interrupt or DMA. 

    if (1)   // If new data has been read 
    {
        // This does only a single byte at a time. 
        if (mavlink_parse_char(
                mavlink_data.channel, 
                mavlink_data.msg_buff[mavlink_data.msg_buff_index], 
                &mavlink_data.msg, 
                &mavlink_data.status))
        {
            // Message received 
            // Decode the message 
        }

        // When do we increment in the index? 
    }
}

//=======================================================================================
