/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2014 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This software is copyrighted by and is the sole property of Express   */ 
/*  Logic, Inc.  All rights, title, ownership, or other interests         */ 
/*  in the software remain the property of Express Logic, Inc.  This      */ 
/*  software may only be used in accordance with the corresponding        */ 
/*  license agreement.  Any unauthorized use, duplication, transmission,  */ 
/*  distribution, or disclosure of this software is expressly forbidden.  */ 
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */ 
/*  written consent of Express Logic, Inc.                                */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               www.expresslogic.com          */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

/***********************************************************************************************************************
 * Copyright [2017] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** USBX Component                                                        */ 
/**                                                                       */
/**   SYNERGY Controller Driver                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_hcd_synergy.h"
#include "ux_host_stack.h"


/*******************************************************************************************************************//**
 * @addtogroup sf_el_ux
 * @{
 **********************************************************************************************************************/

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    ux_hcd_synergy_port_reset                           PORTABLE C      */ 
/*                                                           5.7 SP7      */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Thierry Giron, Express Logic Inc.                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*     This function will reset a specific port attached to the root      */ 
/*     HUB.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_synergy                           Pointer to Synergy controller */ 
/*    port_index                            Port index                    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Synergy Controller Driver                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  10-10-2012     TCRG                     Initial Version 5.6           */ 
/*  06-01-2014     TCRG                     Modified comment(s),          */ 
/*                                            resulting in version 5.7    */ 
/*  12-15-2016     TCRG                     Modified comment(s),          */ 
/*                                            added new error trap        */
/*                                            function support,           */
/*                                            resulting in version 5.7 SP7*/ 
/*                                                                        */ 
/**************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  This function will reset a specific port attached to the root HUB.
 *
 * @param[in]  hcd_synergy : Pointer to a HCD control block
 * @param[in]  port_index  : Port Index
 *
 * @retval UX_SUCCESS              Port reset successfully.
 * @retval UX_PORT_INDEX_UNKNOWN   Invalid port.
 * @retval UX_NO_DEVICE_CONNECTED  Device not connected properly.
 **********************************************************************************************************************/
UINT  ux_hcd_synergy_port_reset(UX_HCD_SYNERGY *hcd_synergy, ULONG port_index)
{
    ULONG       synergy_register_port_status;
    ULONG       reset_in_progress;

    /* Check to see if this port is valid on this controller.  */
    if (hcd_synergy -> ux_hcd_synergy_nb_root_hubs < (UINT)port_index)
    {
        /* Error trap. */
        _ux_system_error_handler((UINT)UX_SYSTEM_LEVEL_THREAD, (UINT)UX_SYSTEM_CONTEXT_HCD, (UINT)UX_PORT_INDEX_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_PORT_INDEX_UNKNOWN, port_index, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return (UINT)UX_PORT_INDEX_UNKNOWN;
    }

    /* Ensure that the downstream port has a device attached. It is unnatural
    to perform a port reset if there is no device.  */
    synergy_register_port_status =  ux_hcd_synergy_port_status_get(hcd_synergy, port_index);

    /* Check Device Connection Status.  */
    if ((synergy_register_port_status &  UX_PS_CCS) == 0)
    {
        /* Error trap. */
        _ux_system_error_handler((UINT)UX_SYSTEM_LEVEL_THREAD, (UINT)UX_SYSTEM_CONTEXT_HCD, (UINT)UX_NO_DEVICE_CONNECTED);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_NO_DEVICE_CONNECTED, port_index, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return (UINT)UX_NO_DEVICE_CONNECTED;
    }

    /* Now we can safely issue a RESET command to this port.  */
    ux_hcd_synergy_register_set(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0, UX_SYNERGY_HC_DVSTCTR0_USBRST);
    ux_hcd_synergy_register_clear(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0, UX_SYNERGY_HC_DVSTCTR0_UACT);

    /* Perform the necessary delay before resetting the port.  */
    tx_thread_sleep(ux_ms_to_ticks(50U));

    /* Now we can safely complete a RESET command to this port.  */
    ux_hcd_synergy_register_set(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0, UX_SYNERGY_HC_DVSTCTR0_UACT);
    ux_hcd_synergy_register_clear(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0, UX_SYNERGY_HC_DVSTCTR0_USBRST);

    tx_thread_sleep(ux_ms_to_ticks(20U));

    reset_in_progress = ux_hcd_synergy_register_read(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0);

    /* Check if reset is completed.  */
    while (reset_in_progress & (ULONG)UX_SYNERGY_HC_DVSTCTR0_RESET_IN_PROGRESS)
    {
        /* Not yet, so wait a bit.  */
        tx_thread_sleep(ux_ms_to_ticks(10U));

        /* Read the reset status again.  */
        reset_in_progress = ux_hcd_synergy_register_read(hcd_synergy, UX_SYNERGY_HC_DVSTCTR0);
    }

    return (UINT)UX_SUCCESS;
}
 /*******************************************************************************************************************//**
  * @} (end addtogroup sf_el_ux)
  **********************************************************************************************************************/

