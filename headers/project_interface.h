/**
 * @file project_interface.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Project initialization and application code interface 
 * 
 * @version 0.1
 * @date 2024-03-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _PROJECT_INTERFACE_H_ 
#define _PROJECT_INTERFACE_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Prototypes 

/**
 * @brief Project initialization code - called once 
 */
void project_init(void); 


/**
 * @brief Project application code - called repeatedly 
 */
void project_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _PROJECT_INTERFACE_H_ 
