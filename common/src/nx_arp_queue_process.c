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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_queue_process                               PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the received ARP messages on the ARP        */
/*    queue placed there by nx_arp_deferred_receive.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_arp_packet_receive                Process received ARP packet   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
VOID  _nx_arp_queue_process(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_PACKET *packet_ptr;


    /* Loop to process all ARP deferred packet requests.  */
    while (ip_ptr -> nx_ip_arp_deferred_received_packet_head)
    {

        /* Remove the first packet and process it!  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Pickup the first packet.  */
        packet_ptr =  ip_ptr -> nx_ip_arp_deferred_received_packet_head;

        /* Move the head pointer to the next packet.  */
        ip_ptr -> nx_ip_arp_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;

        /* Check for end of ARP deferred processing queue.  */
        if (ip_ptr -> nx_ip_arp_deferred_received_packet_head == NX_NULL)
        {

            /* Yes, the ARP deferred queue is empty.  Set the tail pointer to NULL.  */
            ip_ptr -> nx_ip_arp_deferred_received_packet_tail =  NX_NULL;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call the actual ARP packet receive function.  */
        _nx_arp_packet_receive(ip_ptr, packet_ptr);
    }
}

