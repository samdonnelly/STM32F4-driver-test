/**
 * @file driver_test_interface.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver test initialization and application code interface 
 * 
 * @version 0.1
 * @date 2024-03-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _DRIVER_TEST_INTERFACE_H_ 
#define _DRIVER_TEST_INTERFACE_H_ 

//=======================================================================================
// Driver test interface 

/**
 * @brief Project initialization code - called once 
 */
void ProjectInit(void); 


/**
 * @brief Project application code - called repeatedly 
 */
void ProjectApp(void); 


class IDriverTestInterface 
{
public: 
    /**
     * @brief Constructor 
     */
    IDriverTestInterface() = default; 

    /**
     * @brief Destructor 
     */
    ~IDriverTestInterface() = default; 

    // Delete copy constructor and assignment operator
    IDriverTestInterface(const IDriverTestInterface &) = delete;
    IDriverTestInterface &operator=(const IDriverTestInterface &) = delete;

    // Delete move constructor and assignment operator
    IDriverTestInterface(IDriverTestInterface &&) = delete;
    IDriverTestInterface &operator=(IDriverTestInterface &&) = delete;

    /**
     * @brief Initialization code - called once 
     */
    virtual void TestInit(void) noexcept = 0; 

    /**
     * @brief Application code - called repeatedly (or once for RTOS testing) 
     */
    virtual void TestApp(void) noexcept = 0; 
};

//=======================================================================================

#endif   // _DRIVER_TEST_INTERFACE_H_ 
