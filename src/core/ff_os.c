/**
 * @file ff_os.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FatFs OS dependent controls 
 * 
 * @details The below code is taken from the CHAN distribution of FatFs (syscall.c). The 
 *          notes from the original file are below: 
 * 
 * ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics. All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                       opensource.org/licenses/BSD-3-Clause
 *
 * *****************************************************************************
 * 
 * @version 0.1
 * @date 2025-10-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "ff.h"
#include "ffconf.h"

//=======================================================================================


//=======================================================================================
// Synchronization handlers

#if _FS_REENTRANT

/**
 * @brief Create a Synchronization Object 
 * 
 * @details This function is called in f_mount() function to create a new synchronization 
 *          object, such as semaphore and mutex. When a 0 is returned, the f_mount() 
 *          function fails with FR_INT_ERR.
 * 
 * @param vol : Corresponding volume (logical drive number) 
 * @param _SYNC_t : Pointer to return the created sync object 
 * @return int : 1:Function succeeded, 0:Could not create the sync object
 */
int ff_cre_syncobj(
	BYTE vol,
	_SYNC_t *sobj)
{
//     int ret;
// #if _USE_MUTEX

// #if (osCMSIS < 0x20000U)
//     osMutexDef(MTX);
//     *sobj = osMutexCreate(osMutex(MTX));
// #else
//     *sobj = osMutexNew(NULL);
// #endif

// #else

// #if (osCMSIS < 0x20000U)
//     osSemaphoreDef(SEM);
//     *sobj = osSemaphoreCreate(osSemaphore(SEM), 1);
// #else
//     *sobj = osSemaphoreNew(1, 1, NULL);
// #endif

// #endif
//     ret = (*sobj != NULL);

//     return ret;

    return 0;
}


/**
 * @brief Delete a Synchronization Object 
 * 
 * @details This function is called in f_mount() function to delete a synchronization 
 *          object that created with ff_cre_syncobj() function. When a 0 is returned, 
 *          the f_mount() function fails with FR_INT_ERR. 
 * 
 * @param _SYNC_t : Sync object tied to the logical drive to be deleted 
 * @return int : 1:Function succeeded, 0:Could not delete due to any error 
 */
int ff_del_syncobj(_SYNC_t sobj)
{
// #if _USE_MUTEX
//     osMutexDelete (sobj);
// #else
//     osSemaphoreDelete (sobj);
// #endif
    return 1;
}


/**
 * @brief Request Grant to Access the Volume 
 * 
 * @details This function is called on entering file functions to lock the volume. 
 *          When a 0 is returned, the file function fails with FR_TIMEOUT. 
 * 
 * @param _SYNC_t : Sync object to wait 
 * @return int : 1:Got a grant to access the volume, 0:Could not get a grant 
 */
int ff_req_grant(_SYNC_t sobj)
{
//     int ret = 0;
// #if (osCMSIS < 0x20000U)

// #if _USE_MUTEX
//     if(osMutexWait(sobj, _FS_TIMEOUT) == osOK)
// #else
//     if(osSemaphoreWait(sobj, _FS_TIMEOUT) == osOK)
// #endif

// #else

// #if _USE_MUTEX
//     if(osMutexAcquire(sobj, _FS_TIMEOUT) == osOK)
// #else
//     if(osSemaphoreAcquire(sobj, _FS_TIMEOUT) == osOK)
// #endif

// #endif
//     {
//         ret = 1;
//     }

//     return ret;

    return 0;
}


/**
 * @brief Release Grant to Access the Volume 
 * 
 * @details This function is called on leaving file functions to unlock the volume. 
 * 
 * @param _SYNC_t : Sync object to be signaled 
 */
void ff_rel_grant(_SYNC_t sobj)
{
// #if _USE_MUTEX
//     osMutexRelease(sobj);
// #else
//     osSemaphoreRelease(sobj);
// #endif
}

#endif

//=======================================================================================


//=======================================================================================
// Memory control 

#if _USE_LFN == 3	/* LFN with a working buffer on the heap */

/**
 * @brief Allocate a memory block 
 * 
 * @details If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE. 
 * 
 * @param msize : Number of bytes to allocate 
 * @return void : Returns pointer to the allocated memory block 
 */
void* ff_memalloc(UINT msize)
{
	return ff_malloc(msize);	/* Allocate a new memory block with POSIX API */
}


/**
 * @brief Free a memory block 
 * 
 * @param mblock : Pointer to the memory block to free 
 */
void ff_memfree(void* mblock)
{
	ff_free(mblock);	/* Discard the memory block with POSIX API */
}

#endif

//=======================================================================================
