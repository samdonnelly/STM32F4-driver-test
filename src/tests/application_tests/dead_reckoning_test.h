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
// Prototypes 

class DeadReckoningTest final
{
public: 
    
    /**
     * @brief Constructor 
     */
    DeadReckoningTest();

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
     * @brief Dead reckoning test initialization and setup code 
     */
    void TestInit(void); 

    /**
     * @brief Dead reckoning test application code 
     */
    void TestApp(void); 

private: 

    /**
     * @brief Fault state for the IMU 
     */
    void IMUFault(void); 
    
    // Peripherals 
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_periodic; 

    // IMU data 
    device_number_t device_num; 
    uint8_t st_result; 
    MPU6050_STATUS status; 
};

extern DeadReckoningTest dead_reckoning; 

//=======================================================================================

#endif   // _DEAD_RECKONING_TEST_H_ 
