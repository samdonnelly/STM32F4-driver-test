/**
 * @file dead_reckoning_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Dead reckoning test interface 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _DEAD_RECKONING_TEST_H_ 
#define _DEAD_RECKONING_TEST_H_ 

//=======================================================================================
// Includes 

#include "driver_test_interface.h" 
#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Test data and interface 

class DeadReckoningTest final : public IDriverTestInterface 
{
public: 
    /**
     * @brief Constructor 
     */
    DeadReckoningTest() = default; 

    /**
     * @brief Destructor 
     */
    ~DeadReckoningTest() = default; 

    // Delete copy constructor and assignment operator
    DeadReckoningTest(const DeadReckoningTest &) = delete;
    DeadReckoningTest &operator=(const DeadReckoningTest &) = delete;

    // Delete move constructor and assignment operator
    DeadReckoningTest(DeadReckoningTest &&) = delete;
    DeadReckoningTest &operator=(DeadReckoningTest &&) = delete;

    /**
     * @brief Initialization code - called once 
     */
    void TestInit(void) noexcept override; 

    /**
     * @brief Application code - called repeatedly 
     */
    void TestApp(void) noexcept override; 
};

extern DeadReckoningTest dead_reckoning; 

//=======================================================================================

#endif   // _DEAD_RECKONING_TEST_H_ 
