/**
 * @file state_machine_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief State machine test code interface 
 * 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _STATE_MACHINE_TEST_H_ 
#define _STATE_MACHINE_TEST_H_ 

//================================================================================
// Includes 

#include "includes_drivers.h"

//================================================================================


//================================================================================
// How to use this test code: 
// - This code is used to test device controller, in particular the performance of their state
//   machine. When set up correctly, the user enters commands via a serial terminal and the test 
//   code matches user input to user defined commands to control setters and getters in the 
//   controllers. 
// 
// - First, define an instance of state_request_t in the test code in the form of an array. Each 
//   entry is a command and will correspond to a function pointer that points to a setter or 
//   getter. The function pointers are defined in a separate array because the array format 
//   is unique to a device test/controller. For each state_request_t array entry define what 
//   string to look for from the user, the number of arguments in the associated function pointer, 
//   the function pointer index in the function pointer array, and the function argument buffer 
//   index. See state_request_t below for more info. 
//   
//   Ex: 
//       --> static state_request_t state_cmds[HD44780U_NUM_USER_CMDS] = {
//              {"user_cmd_1", 2, SETTER_PTR_2, 0}, ... }; 
//   
//   It's important to note that the last defined entry of this array must be: 
//       --> {"execute", 0, 0, 0} (can replace "execute" with a command word of choice) 
//   During operation the user will input various commands at the terminal. These commands are 
//   all recorded by the code and will not execute until told to do so. This final command 
//   essentially tells the code to "go" at which point it will run all the input commands by 
//   calling setters and getters, then the controller will be run based on these input and the 
//   controller behavior can be observed. After running the execute command then user input data 
//   gets reset and must be entered again. 
// 
// - Second, define all the needed function pointers in the device test code header file. After 
//   those are defined then create a structure containing all the function pointers. In the device 
//   test code create an instance of the structure as an array and define all the function 
//   pointers to getters and setters within it. It's important to note that this array must match 
//   the state_request_t array in size as the information defined there directly related to this 
//   array. The function pointer index from state_request_t defines where to look in this array 
//   to get the function pointer. 
//   
//   Ex. of how this array is defined: 
//       --> static device_func_ptrs_t state_func[NUM_USER_CMDS] = {
//              {NULL, &device_data_set, NULL}, ... }; 
// 
// - Call state_machine_init in the setup code. 
// 
// - Call state_machine_test repeatedly in the test code. 
// 
// - After calling state_machine_test, you can use the set_get_status to figure out which setters 
//   and getters have been requested. Each bit in this variable corresponds to a setter or 
//   getter and each bit should be checked. When a bit (setter or getter) is seen to be set 
//   (requested) then the appropriate function pointer can be called by reading the pre-defined 
//   func_ptr_index. 
// 
// - After calling state_machine_test, you can use the arg_convert flag to know when to convert 
//   a user input into usable function arguments. If arg_convert is set then it means the user 
//   has input all needed arguments. If arg_convert is set then the func_ptr_index can be used 
//   to identify how to convert the inputs to match the arguments for the function pointer. 
// 
// - Once all of this is implemented the code can be run. Within the serial terminal you can 
//   type the commands that you defined in the code. The test code will match your input to 
//   the pre-defined commands and store these requsts. Once all the desired inputs have been 
//   set then the "execute" command can be run to make the change in the setters and getters 
//   then run the contoller. From here the controller behavior can be observed. 
// 
//================================================================================


//================================================================================
// Macros 

#define STATE_TEST_CMD_LEN 15         // Max user command length 
#define STATE_USER_TEST_INPUT 30      // User input buffer size 

//================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Used to specify the number of function pointer arguments 
 * 
 * @see state_request_t 
 */
typedef enum {
    SMT_ARGS_0, 
    SMT_ARGS_1, 
    SMT_ARGS_2, 
    SMT_ARGS_3, 
    SMT_ARGS_4 
} smt_arg_num_t; 


/**
 * @brief Function pointer index 
 * 
 * @details This enum is used to define func_ptr_index within state_request_t. An array of 
 *          function pointers is created within the test code (see description above) and 
 *          this enum index must match the position of that function pointer within the 
 *          array. 
 * 
 * @see state_request_t 
 */
typedef enum {
    SMT_STATE_FUNC_PTR_1, 
    SMT_STATE_FUNC_PTR_2, 
    SMT_STATE_FUNC_PTR_3, 
    SMT_STATE_FUNC_PTR_4, 
    SMT_STATE_FUNC_PTR_5, 
    SMT_STATE_FUNC_PTR_6, 
    SMT_STATE_FUNC_PTR_7, 
    SMT_STATE_FUNC_PTR_8, 
    SMT_STATE_FUNC_PTR_9 
} smt_state_func_ptr_index_t; 


/**
 * @brief Used to specify the position in the user defined argument buffer 
 * 
 * @see state_request_t 
 */
typedef enum {
    SMT_ARG_BUFF_POS_0, 
    SMT_ARG_BUFF_POS_1, 
    SMT_ARG_BUFF_POS_2, 
    SMT_ARG_BUFF_POS_3 
} smt_arg_index_t; 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief State machine test user defined commands 
 * 
 * @details This structures defined the user commands used for the state machine testing. An 
 *          instance of this structure as an array is defined in the device test code file and 
 *          is comprised of the following information: <br> 
 *          
 *          cmd : user input at the serial terminal that triggers a command. <br> 
 *          
 *          arg_num : number of arguments in the function associated with a command. <br> 
 *          
 *          func_ptr_index : position of where to find appropriate function pointer in user 
 *                           defined function pointer array. <br> 
 *          
 *          arg_buff_index : Some setters in the device controller take arguments. Sometimes 
 *                           there are multiple setters with the same arguments. When a setter 
 *                           takes arguments the code has to store the user input that defines 
 *                           those arguments. Instead of defining individual input buffers for 
 *                           each setter with user input arguments, one buffer array is defined 
 *                           and this parameter indicated the index in that array of where to 
 *                           find the correct arguments. 
 *                           All the items that have the same func_ptr_index will share the 
 *                           same argument buffer so they should all have unique arg_buff_index
 *                           starting with 0. 
 * 
 */
typedef struct state_request_s 
{
    char cmd[STATE_TEST_CMD_LEN];        // Stores the possible user commands 
    uint8_t arg_num;                     // Number of arguments in function 
    uint8_t func_ptr_index;              // Which generic function pointer to use 
    uint8_t arg_buff_index;              // Position in user defined argument buffer 
}
state_request_t; 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief State machine tester initialization 
 * 
 * @details Pre-defines the parameters of state_test_params_t needed for the state machine 
 *          control. The only argument is the number of user defined commands. This will 
 *          depend on the commands defined by the user which are specific to a device test. 
 * 
 * @see state_test_params_t
 * 
 * @param num_usr_cmds : number of user defined commands 
 */
void state_machine_init(uint8_t num_usr_cmds); 


/**
 * @brief State machine tester 
 * 
 * @details General purpose test code used to test state machines in device controllers. If 
 *          set up properly (see how-to above) then the code allows for user input on a serial 
 *          terminal and takes that input to control a device state machine. 
 * 
 * @param state_request : pointer to array of possible user commands - defined in driver test 
 * @param user_args : pointer to buffer for storing user terminal input 
 * @param cmd_index : pointer to a variable for returning the command index 
 * @param arg_convert : pointer to a variable for returning the argument convert flag 
 * @param set_get_status : pointer to a variable for returning the setter status 
 */
void state_machine_test(
    state_request_t state_request[], 
    char *user_args, 
    uint8_t *cmd_index, 
    uint8_t *arg_convert, 
    uint32_t *set_get_status); 

//=======================================================================================

#endif   // _STATE_MACHINE_TEST_H_
