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

//=======================================================================================
// Includes 

#include "includes_app.h"
#include "includes_cpp_app.h"
#include "includes_drivers.h"
#include "includes_cpp_drivers.h"

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Project initialization code - called once 
 */
void ProjectInit(void); 


/**
 * @brief Project application code - called repeatedly 
 */
void ProjectApp(void); 


class IProjectInterface 
{
public: 
    // Constructor/Destructor 
    IProjectInterface() = default; 
    ~IProjectInterface() = default; 

    // Initialization code - called once 
    virtual void ProjectInit(void) = 0; 

    // Application code - called repeatedly (or once for RTOS testing) 
    virtual void ProjectApp(void) = 0; 
};


class ProjectInterface 
{
public: 
    IProjectInterface &project; 

public: 
    ProjectInterface(IProjectInterface &interface) : project(interface) {}
};

//=======================================================================================

#endif   // _PROJECT_INTERFACE_H_ 
