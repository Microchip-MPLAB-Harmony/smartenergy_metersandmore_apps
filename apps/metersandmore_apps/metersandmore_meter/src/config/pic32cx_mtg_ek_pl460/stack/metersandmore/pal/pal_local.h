/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_local.h

  Summary:
    Platform Abstraction Layer (PAL) Local header file.

  Description:
    Platform Abstraction Layer (PAL) Local header file. This file
    provides definitions and types internally used by PAL.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

#ifndef PAL_LOCAL_H
#define PAL_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "system/system.h"
#include "driver/driver.h"
#include "service/pcoup/srv_pcoup.h"
#include "pal.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PAL states

  Summary:
    PAL states enumeration

  Description:
    This enumeration defines the valid PAL states. These states
    determine the behavior of the PAL at different stages.
*/

typedef enum
{
    PAL_STATE_IDLE=0,
    PAL_STATE_INIT,
    PAL_STATE_OPEN,
    PAL_STATE_WAIT_PVDD_MON,
    PAL_STATE_ERROR

} PAL_STATE;

// *****************************************************************************
/* PAL Data

  Summary:
    Holds PAL internal data.

  Description:
    This data type defines all data required to handle the PAL module.

  Remarks:
    None.
*/
typedef struct
{
    DRV_HANDLE drvHandle;

    PAL_HANDLERS initHandlers;

    PAL_STATUS status;

    PAL_STATE state;

    SRV_PLC_PCOUP_BRANCH plcBranch;

    uint8_t statsErrorUnexpectedKey;

    uint8_t statsErrorReset;

    uint8_t statsErrorDebug;

    uint8_t statsErrorCritical;

    bool waitingTxCfm;

    bool pvddMonTxEnable;

} PAL_DATA;

#endif // #ifndef PAL_LOCAL_H
/*******************************************************************************
 End of File
*/