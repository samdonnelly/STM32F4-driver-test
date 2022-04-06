/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_app.h"

//=======================================================================================


// User application 
void user_app()
{
    // 
    static uint8_t data_list[4];

    // Delay 
    tim9_delay_ms(1000);

    // Test print 
    data_list[0] = (0x49 & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    data_list[1] = (0x49 & 0xF0) | HD44780U_CONFIG_CMD_0X09;
    data_list[2] = ((0x49 << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    data_list[3] = ((0x49 << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X09;
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
}
