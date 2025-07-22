/**
 * @file hw125_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 test code interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW125_TEST_H_
#define _HW125_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Structs 

// SD card data 
typedef struct hw125_test_drive_data_s 
{
    // File variables 
    FATFS   file_sys;                     // File system 
    FIL     file;                         // File 
    FRESULT fresult;                      // Store the result of each operation 
    UINT    br, bw;                       // Stores f_read and f_write byte counters 
    DIR     dj;                           // Directory object 
    FILINFO fno;                          // File information 

    // Card capacity 
    FATFS *pfs;                           // Pointer to file system object 
    DWORD fre_clust;                      // Stores number of free clusters 
    DWORD total, free_space;              // Total and free volume space 
} 
hw125_test_drive_data_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HW125 setup code 
 */
void hw125_test_init(void); 


/**
 * @brief HW125 test code 
 */
void hw125_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HW125_TEST_H_ 
