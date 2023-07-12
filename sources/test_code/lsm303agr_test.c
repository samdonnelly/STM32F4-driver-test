/**
 * @file lsm303agr_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_test.h" 

//=======================================================================================


//=======================================================================================
// 

void lsm303agr_init(
    I2C_TypeDef *, 
    lsm303agr_m_odr_cfg_t, 
    lsm303agr_m_sys_mode_t, 
    lsm303agr_cfg_t, 
    lsm303agr_cfg_t, 
    lsm303agr_cfg_t, 
    lsm303agr_cfg_t); 

//=======================================================================================


// Setup code
void lsm303agr_test_init(void)
{
    // Setup code for the gpio_test here 

    lsm303agr_init(
        I2C1, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
} 


// Test code 
void lsm303agr_test_app(void)
{
    // Test code for the gpio_test here 
}
