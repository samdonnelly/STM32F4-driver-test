/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
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
#include "string.h"
#include "stdio.h"

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


// User application 
void user_app()
{
    // Looped code here

    // Local variables 
    static uint8_t count = 1; 

    // Run once - code is here to put everything in the same file 
    if (count)
    {
        // Short delay to let the system set up 
        tim9_delay_ms(500); 

#if FORMAT_EXFAT
        // TODO test to see if this will erase existing data 
        // Format the drive 
        fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
        if (fresult != FR_OK) uart2_sendstring("Error in formatting the SD Card.\r\n");
        else uart2_sendstring("SD Card formatted successfully.\r\n"); 
#endif

        //=============================================
        // Mount the SD card 

        fresult = f_mount(&file_sys, "", 1); 
        if (fresult != FR_OK) uart2_sendstring("Error in mounting SD Card.\r\n");
        else uart2_sendstring("SD Card mounted successfully.\r\n"); 

        //=============================================


        //=============================================
        // Card Capacity 

        // These calcs assume 512 bytes/sector 

        // Check free space 
        f_getfree("", &fre_clust, &pfs);

        total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Total Size: \t%lu KB\r\n", total);
        uart2_sendstring(buffer);
        buff_clear();
        
        free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Free Space: \t%lu KB\r\n\n", free_space);
        uart2_sendstring(buffer);
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
        if (fresult == FR_OK) uart2_sendstring("test_file.txt successfully written.\r\n\n"); 
        else uart2_sendstring("Problems writing data to test_file.txt.\r\n\n"); 

        // Open the file again to read the data 
        fresult = f_open(&file, "test_file.txt", FA_READ); 

        // Read the string from the file 
        f_gets(buffer, f_size(&file), &file); 

        // Display the data 
        uart2_sendstring("test_file.txt has been read and the string inside says: \r\n"); 
        uart2_sendstring(buffer); 
        uart2_send_new_line(); 
        uart2_send_new_line(); 

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
        uart2_sendstring("test_file_2.txt written to.\r\n"); 
        f_close(&file); 

        // Clear the buffer so we can clearly see the contents read from the file 
        buff_clear(); 

        // Open the file 
        fresult = f_open(&file, "test_file_2.txt", FA_READ); 
        if (fresult == FR_OK) uart2_sendstring("test_file_2.txt successfully opened.\r\n"); 

        // Read the data from the file 
        uart2_sendstring("test_file_2.txt contents: \r\n"); 
        uart2_sendstring("\t"); 
        f_read(&file, buffer, f_size(&file), &br); 
        uart2_sendstring(buffer); 
        uart2_send_new_line(); 
        uart2_send_new_line(); 

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
        if (fresult == FR_OK) uart2_sendstring("test_file_2.txt will be appended\r\n"); 

        // Write to the end of the file 
        fresult = f_puts("Appended data.\r\n", &file); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        // Open the file to read the contents 
        fresult = f_open(&file, "test_file_2.txt", FA_READ); 

        // Read the data on the file 
        fresult = f_read(&file, buffer, f_size(&file), &br); 
        if (fresult == FR_OK) uart2_sendstring("test_file_2.txt updated data: \r\n\t"); 
        uart2_sendstring(buffer); 
        uart2_send_new_line(); 

        // Close the file 
        f_close(&file); 
        buff_clear(); 

        //============================================= 


        //============================================= 
        // Remove files from the drive 

        fresult = f_unlink("/test_file.txt"); 
        if (fresult == FR_OK) uart2_sendstring("test_file.txt removed successfully.\r\n"); 

        fresult = f_unlink("/test_file_2.txt"); 
        if (fresult == FR_OK) uart2_sendstring("test_file_2.txt removed successfully.\r\n"); 

        //============================================= 


        //============================================= 
        // Unmount the SD card 

        fresult = f_mount(NULL, "", 1); 
        if (fresult == FR_OK) uart2_sendstring("SD unmounted successfully.\r\n"); 
        
        //============================================= 

        count = 0; 
    }

    // Delay 
    tim9_delay_ms(1);
}


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
