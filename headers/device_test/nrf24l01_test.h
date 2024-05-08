/**
 * @file nrf24l01_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module test code header 
 * 
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NRF24L01_TEST_H_ 
#define _NRF24L01_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define NRF24L01_TEST_MAX_INPUT 32   // Max user input command length (bytes) 

//=======================================================================================


//=======================================================================================
// Enums 

typedef enum {
    NRF24L01_CMD_ARG_NONE, 
    NRF24L01_CMD_ARG_VALUE, 
    NRF24L01_CMD_ARG_STR 
} nrf24l01_cmd_arg_t; 

//=======================================================================================


//=======================================================================================
// Structs 

// Command template 
typedef struct nrf24l01_cmds_s 
{
    const char *user_cmds; 
    void (*cmd_ptr)(uint8_t, uint8_t *); 
}
nrf24l01_cmds_t; 


// User command data 
typedef struct nrf24l01_cmd_data_s 
{
    uint8_t cb[NRF24L01_TEST_MAX_INPUT];         // Circular buffer (CB) for user inputs 
    uint8_t cb_index;                            // CB index used for parsing commands 
    uint8_t cmd_buff[NRF24L01_TEST_MAX_INPUT];   // User command parsed from the CB 
    uint8_t cmd_id[NRF24L01_TEST_MAX_INPUT];     // ID from the user command 
    uint8_t cmd_value;                           // Value from the user command 
    uint8_t cmd_str[NRF24L01_TEST_MAX_INPUT];    // String from the user command 
}
nrf24l01_cmd_data_t; 

//=======================================================================================


//=======================================================================================
// Test code 

/**
 * @brief nRF24L01 setup code 
 */
void nrf24l01_test_init(void); 


/**
 * @brief nRF24L01 test code 
 */
void nrf24l01_test_app(void); 

//=======================================================================================


//=======================================================================================
// Test functions 

/**
 * @brief Check for user input and execute callbacks if a valid command arrives 
 * 
 * @param cmd_data : user command info 
 * @param cmd_table : list of commands and callbacks 
 * @param num_cmds : number of commands 
 * @param cmd_arg_type : argument type to look for 
 */
void nrf24l01_test_user_input(
    nrf24l01_cmd_data_t *cmd_data, 
    const nrf24l01_cmds_t *cmd_table, 
    uint8_t num_cmds, 
    nrf24l01_cmd_arg_t cmd_arg_type); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_TEST_H_ 
