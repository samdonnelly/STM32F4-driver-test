/**
 * @file hw125_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "hw125_test.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

// To find the size of data in the buffer 
int buff_size(char *buff); 

// Clear the buffer 
void buff_clear(void); 

//=======================================================================================


//=======================================================================================
// FATFS variables 

FATFS   file_sys;            // File system 
FIL     file;                // File 
FRESULT fresult;             // Store the result of each operation 
char    buffer[BUFF_SIZE];   // To store the data that we can read or write
UINT    br, bw;              // Stores the counter to the read and write in the file 

// Format drive variables 
BYTE    work[512];           // 

// Capacity related variables 
FATFS    *pfs; 
DWORD    fre_clust; 
uint32_t total, free_space; 

//=======================================================================================


// Setup code
void hw125_test_init()
{
    // Setup code for the hw125_test here 

    // Local variables 
    // uint8_t card_type; 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // UART2 for serial terminal communication 
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 

    // SPI 
    spi_init(SPI2, GPIOB, SPI2_1_SLAVE, BR_FPCLK_8, SPI_CLOCK_MODE_0);

    // Device initialization 
    // TODO probably have to specify the GPIO port here 
    hw125_user_init(GPIOX_PIN_9);
    
    // card_type = hw125_init(GPIOB_PIN_9);

    // // Check the SD card initialization status 
    // switch (card_type)
    // {
    //     case HW125_CT_UNKNOWN:  // Error occured 
    //         uart_sendstring(USART2, "Error: Unknown card\r\n");
    //         break;
    //     case HW125_CT_MMC:  // MMC V3 
    //         uart_sendstring(USART2, "MMC V3\r\n");
    //         break;
    //     case HW125_CT_SDC1:  // SDC V1 
    //         uart_sendstring(USART2, "SDC V1\r\n");
    //         break;
    //     case HW125_CT_SDC2_BLOCK:  // SDC V2 block 
    //         uart_sendstring(USART2, "SDC V2 block\r\n");
    //         break;
    //     case HW125_CT_SDC2_BYTE:  // SDC V2 byte 
    //         uart_sendstring(USART2, "SDC V2 byte\r\n");
    //         break;
    //     default:  // Unknown response 
    //         uart_sendstring(USART2, "Unknown response\r\n");
    //         break;
    // }
} 


// Test code 
void hw125_test_app()
{
    // Test code for the hw125_test here 

    // Local variables 
    static uint8_t count = 1; 

    // Run once - code is here to put everything in the same file 
    if (count)
    {
        // Short delay to let the system set up 
        tim_delay_ms(TIM9, 500); 

#if FORMAT_EXFAT
        // TODO test to see if this will erase existing data 
        // Format the drive 
        fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
        if (fresult != FR_OK) uart_sendstring(USART2, "Error in formatting the SD Card.\r\n");
        else uart_sendstring(USART2, "SD Card formatted successfully.\r\n"); 
#endif

        //=============================================
        // Mount the SD card 

        fresult = f_mount(&file_sys, "", 1); 
        if (fresult != FR_OK) uart_sendstring(USART2, "Error in mounting SD Card.\r\n");
        else uart_sendstring(USART2, "SD Card mounted successfully.\r\n"); 

        //=============================================


        //=============================================
        // Card Capacity 

        // These calcs assume 512 bytes/sector 

        // Check free space 
        f_getfree("", &fre_clust, &pfs);

        total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Total Size: \t%lu KB\r\n", total);
        uart_sendstring(USART2, buffer);
        buff_clear();
        
        free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Free Space: \t%lu KB\r\n\n", free_space);
        uart_sendstring(USART2, buffer);
        buff_clear();

        //=============================================


        //=============================================
        // f_puts and f_gets (wrapper functions of f_read and f_write) 

        // Open a file (and create if it doesn't exist) 
        fresult = f_open(&file, "test_file.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE); 

        // Write a string 
        f_puts("This string was written using f_puts.", &file); 

        // Close the file 
        fresult = f_close(&file); 

        // Check the status of the operation 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file.txt successfully written.\r\n\n"); 
        else uart_sendstring(USART2, "Problems writing data to test_file.txt.\r\n\n"); 

        // Open the file again to read the data 
        fresult = f_open(&file, "test_file.txt", FA_READ); 

        // Read the string from the file 
        f_gets(buffer, f_size(&file), &file); 

        // Display the data 
        uart_sendstring(USART2, "test_file.txt has been read and the string inside says: \r\n"); 
        uart_sendstring(USART2, buffer); 
        uart_send_new_line(USART2); 
        uart_send_new_line(USART2); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        //=============================================


        //=============================================
        // f_write and f_read

        // Create/open a second file 
        fresult = f_open(&file, "test_file_2.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE); 

        // Create text to write to the file 
        strcpy(buffer, "This file was written to using f_write and read using r_read.\n"); 

        // Write the text 
        fresult = f_write(&file, buffer, buff_size(buffer), &bw); 

        // Close the file 
        uart_sendstring(USART2, "test_file_2.txt written to.\r\n"); 
        f_close(&file); 

        // Clear the buffer so we can clearly see the contents read from the file 
        buff_clear(); 

        // Open the file 
        fresult = f_open(&file, "test_file_2.txt", FA_READ); 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file_2.txt successfully opened.\r\n"); 

        // Read the data from the file 
        uart_sendstring(USART2, "test_file_2.txt contents: \r\n"); 
        uart_sendstring(USART2, "\t"); 
        f_read(&file, buffer, f_size(&file), &br); 
        uart_sendstring(USART2, buffer); 
        uart_send_new_line(USART2); 
        uart_send_new_line(USART2); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        //=============================================


        //=============================================
        // Update an existing file 

        // Open the file to edit 
        fresult = f_open(&file, "test_file_2.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE); 

        // Move to the end of the file to append data 
        fresult = f_lseek(&file, f_size(&file)); 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file_2.txt will be appended\r\n"); 

        // Write to the end of the file 
        fresult = f_puts("Appended data.\r\n", &file); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        // Open the file to read the contents 
        fresult = f_open(&file, "test_file_2.txt", FA_READ); 

        // Read the data on the file 
        fresult = f_read(&file, buffer, f_size(&file), &br); 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file_2.txt updated data: \r\n\t"); 
        uart_sendstring(USART2, buffer); 
        uart_send_new_line(USART2); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        //============================================= 


        //============================================= 
        // Remove files from the drive 

        fresult = f_unlink("/test_file.txt"); 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file.txt removed successfully.\r\n"); 

        fresult = f_unlink("/test_file_2.txt"); 
        if (fresult == FR_OK) uart_sendstring(USART2, "test_file_2.txt removed successfully.\r\n"); 

        //============================================= 


        //============================================= 
        // Unmount the SD card 

        fresult = f_mount(NULL, "", 1); 
        if (fresult == FR_OK) uart_sendstring(USART2, "SD unmounted successfully.\r\n"); 
        
        //============================================= 

        count = 0; 
    }

    // Delay 
    tim_delay_ms(TIM9, 1);
}


//=======================================================================================
// Test functions 


// To find the size of data in the buffer 
int buff_size(char *buff)
{
    int i = 0; 
    while(*buff++ != '\0') i++; 
    return i; 
}


// Clear the buffer 
void buff_clear(void)
{
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        buffer[i] = '\0';
    }
}

//=======================================================================================
