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
    /**
     * @brief Constructor 
     */
    IProjectInterface() = default; 

    /**
     * @brief Destructor 
     */
    ~IProjectInterface() = default; 

    // Delete copy constructor and assignment operator
    IProjectInterface(const IProjectInterface &) = delete;
    IProjectInterface &operator=(const IProjectInterface &) = delete;

    // Delete move constructor and assignment operator
    IProjectInterface(IProjectInterface &&) = delete;
    IProjectInterface &operator=(IProjectInterface &&) = delete;

    /**
     * @brief Initialization code - called once 
     */
    virtual void ProjectInit(void) noexcept = 0; 

    /**
     * @brief Application code - called repeatedly (or once for RTOS testing) 
     */
    virtual void ProjectApp(void) noexcept = 0; 
};

//=======================================================================================

#endif   // _PROJECT_INTERFACE_H_ 
