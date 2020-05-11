/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_packet_pool_create                             PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the packet pool create           */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Packet Pool control block     */
/*    name_ptr                              Packet Pool string pointer    */
/*    payload_size                          Size of packet payload        */
/*    pool_start                            Starting address of pool      */
/*    pool_size                             Number of bytes in pool       */
/*    pool_control_block_size               Size of packet pool control   */
/*                                            block                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_pool_create                Actual packet pool create     */
/*                                            function                    */
/*    tx_thread_identify                    Get current thread pointer    */
/*    tx_thread_preemption_change           Change preemption for thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name_ptr, ULONG payload_size,
                              VOID *pool_start, ULONG pool_size, UINT pool_control_block_size)
{

UINT            status;
UINT            rounded_payload_size;
UINT            old_threshold;
NX_PACKET_POOL *created_pool;
ULONG           created_count;
CHAR           *end_memory;
CHAR           *created_end;
TX_THREAD      *current_thread;


    /* Check for invalid input pointers.  */
    if ((pool_ptr == NX_NULL) || (pool_start == NX_NULL) || (pool_control_block_size != sizeof(NX_PACKET_POOL)))
    {
        return(NX_PTR_ERROR);
    }

    /* Round the packet size up to something that is evenly divisible by
       an ULONG.  This helps guarantee proper alignment.  */
    rounded_payload_size =  ((payload_size + sizeof(ULONG) - 1) / sizeof(ULONG)) * sizeof(ULONG);

    /* Round the pool size down to something that is evenly divisible by
       an ULONG.  */
    pool_size =   (pool_size / sizeof(ULONG)) * sizeof(ULONG);

    /* Check for an invalid pool and payload size.  */
    if ((!payload_size) || (!pool_size) || (rounded_payload_size > (pool_size - sizeof(NX_PACKET))))
    {
        return(NX_SIZE_ERROR);
    }

    /* Calculate the end of the pool memory area.  */
    end_memory =  ((CHAR *)pool_start) + (pool_size - 1);

    /* Pickup current thread pointer.  */
    current_thread =  tx_thread_identify();

    /* Disable preemption temporarily.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, 0, &old_threshold);
    }

    /* Loop to check for the pool instance already created.  */
    created_pool =   _nx_packet_pool_created_ptr;
    created_count =  _nx_packet_pool_created_count;
    while (created_count--)
    {

        /* Calculate the created pool's end of memory.  */
        created_end =  created_pool -> nx_packet_pool_start + (created_pool -> nx_packet_pool_size - 1);

        /* Is the new pool already created?  */
        if ((pool_ptr == created_pool) ||
            ((pool_start >= (VOID *)created_pool -> nx_packet_pool_start) && (pool_start < (VOID *)created_end)) ||
            ((end_memory  >= created_pool -> nx_packet_pool_start) && (end_memory  < created_end)))
        {

            /* Restore preemption.  */
            if (current_thread)
            {
                tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
            }

            /* Duplicate packet pool created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_pool =  created_pool -> nx_packet_pool_created_next;
    }

    /* Restore preemption.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual packet pool create function.  */
    status =  _nx_packet_pool_create(pool_ptr, name_ptr, payload_size, pool_start, pool_size);

    /* Return completion status.  */
    return(status);
}

