/*******************************************************************************
  Meters and More Host Interface Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    mmhi_mib.c

  Summary:
    Meters and More Management Information Base(MIB) interface implementation.

  Description:
    This file implements the interface rules between the EUT and the Meters and 
    More Testing Tool that the Meters and More Test Provider (MMTP) will use 
    in order to perform Certification Test.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "mmhi.h"
#include "mmhi_mib.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
/* MM Host Interface Data */
static MMHI_MIB_DB mmhiMibData;
static MMHI_MIB_DB mmhiMibDefaultData = {
    {MIB_RX_NORMAL, MIB_TX_NORMAL},
    {0xAAA5, 0, 0, 0, 0, MMHI_MIB_FW_VERSION_MAJOR, MMHI_MIB_FW_VERSION_MINOR},
    {{0}, {{0}}},
    {{0}, 0, 0, 3},
    {MMHI_MIB_TIME_TEL, MMHI_MIB_TIME_DELAY, MMHI_MIB_TIME_ICDELAY, MMHI_MIB_TIME_TC, MMHI_MIB_TIME_TCT}
};
static uint16_t mmhiMibStatus;
static MMHI_MIB_INDEX mmhiMibWriteIndex;
static MMHI_MIB_DATA mmhiMibWriteData;
static MMHI_MIB_WRITE_IND_CALLBACK mmhiMibCallback;

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

static void lMMHI_MIB_GetDefault(MMHI_MIB_DB *mibData)
{
    memcpy(mibData, &mmhiMibDefaultData, sizeof(MMHI_MIB_DB));
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Callbacks
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: System Interface Functions
// *****************************************************************************
// *****************************************************************************

void MMHI_MIB_Initialize(void)
{
    mmhiMibWriteIndex = 0;
    lMMHI_MIB_GetDefault(&mmhiMibData);
    mmhiMibStatus = MMHI_STATUS_MASK_MAC_CFG_MIB_Msk |
            MMHI_STATUS_MASK_FW_RELEASE_MIB_Msk |
            MMHI_STATUS_MASK_MNF_DATA_MIB_Msk |
            MMHI_STATUS_MASK_SCA_MIB_Msk |
            MMHI_STATUS_MASK_TIMING_PARAMS_MIB_Msk;
}

uint16_t MMHI_MIB_GetStatus(void)
{
    return mmhiMibStatus;
}

MMHI_RESULT MMHI_MIB_Get(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData)
{
    MMHI_RESULT result = MMHI_SUCCESS;

    switch (mibIndex)
    {
        case MIB_MIB_ID_MAC_CONFIG:
            pData->dataLength = sizeof(MMHI_MIB_MAC_CONFIG);
            (void)memcpy(pData->dataValue, &mmhiMibData.macConfig, pData->dataLength);
            break;

        case MIB_MIB_ID_FW_RELEASE:
            pData->dataLength = sizeof(MMHI_MIB_FW_VERSION);
            (void)memcpy(pData->dataValue, &mmhiMibData.fwVersion, pData->dataLength);
            break;

        case MIB_MIB_ID_MANUF_DATA:
            pData->dataLength = sizeof(MMHI_MIB_MANUFACTURER_DATA);
            (void)memcpy(pData->dataValue, &mmhiMibData.manufacturer, pData->dataLength);
            break;

        case MIB_MIB_ID_LOGICAL_ADDRESS:
            pData->dataLength = sizeof(MMHI_MIB_LOGICAL_ADDRESS);
            (void)memcpy(pData->dataValue, &mmhiMibData.address, pData->dataLength);
            break;

        case MIB_MIB_ID_TIMING:
            pData->dataLength = sizeof(MMHI_MIB_TIMING_PARAMETERS);
            (void)memcpy(pData->dataValue, &mmhiMibData.timing, pData->dataLength);
            break;

        default:
            result = MMHI_ERROR_WPV;

    }

    return result;
}

MMHI_RESULT MMHI_MIB_Set(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData, bool indEnable)
{
    MMHI_RESULT result = MMHI_SUCCESS;
    uint8_t reqLength;

    reqLength = pData->dataLength;
    
    switch (mibIndex)
    {
        case MIB_MIB_ID_MAC_CONFIG:
        {         
            if (reqLength == sizeof(MMHI_MIB_MAC_CONFIG))
            {
                /* Update Status flags */
                (void)memcpy(&mmhiMibData.macConfig, pData->dataValue, reqLength);
                if (memcmp(&mmhiMibData.macConfig, &mmhiMibDefaultData.macConfig, 
                        sizeof(MMHI_MIB_MAC_CONFIG)) == 0)
                {
                    mmhiMibStatus |= MMHI_STATUS_MASK_MAC_CFG_MIB_Msk;
                }
                else
                {
                    mmhiMibStatus &= ~MMHI_STATUS_MASK_MAC_CFG_MIB_Msk;
                }

                if (indEnable == true)
                {
                    /* Update Write Indication flag */
                    mmhiMibWriteIndex = MIB_MIB_ID_MAC_CONFIG;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_FW_RELEASE:
            /* Read-only */
            result = MMHI_ERROR_WPV;
            break;

        case MIB_MIB_ID_MANUF_DATA:
        {         
            if (reqLength == sizeof(MMHI_MIB_MANUFACTURER_DATA))
            {
                (void)memcpy(&mmhiMibData.manufacturer, pData->dataValue, reqLength);
                if (memcmp(&mmhiMibData.manufacturer, &mmhiMibDefaultData.manufacturer, 
                        sizeof(MMHI_MIB_MANUFACTURER_DATA)) == 0)
                {
                    mmhiMibStatus |= MMHI_STATUS_MASK_MNF_DATA_MIB_Msk;
                }
                else
                {
                    mmhiMibStatus &= ~MMHI_STATUS_MASK_MNF_DATA_MIB_Msk;
                }

                if (indEnable == true)
                {
                    /* Update Write Indication flag */
                    mmhiMibWriteIndex = MIB_MIB_ID_MANUF_DATA;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_LOGICAL_ADDRESS:
        {         
            if (reqLength == sizeof(MMHI_MIB_LOGICAL_ADDRESS))
            {
                (void)memcpy(&mmhiMibData.address, pData->dataValue, reqLength);
                if (memcmp(&mmhiMibData.address, &mmhiMibDefaultData.address, 
                        sizeof(MMHI_MIB_LOGICAL_ADDRESS)) == 0)
                {
                    mmhiMibStatus |= MMHI_STATUS_MASK_SCA_MIB_Msk;
                }
                else
                {
                    mmhiMibStatus &= ~MMHI_STATUS_MASK_SCA_MIB_Msk;
                }

                if (indEnable == true)
                {
                    /* Update Write Indication flag */
                    mmhiMibWriteIndex = MIB_MIB_ID_LOGICAL_ADDRESS;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_TIMING:
        {         
            if (reqLength == sizeof(MMHI_MIB_TIMING_PARAMETERS))
            {
                (void)memcpy(&mmhiMibData.timing, pData->dataValue, reqLength);
                if (memcmp(&mmhiMibData.timing, &mmhiMibDefaultData.timing, 
                        sizeof(MMHI_MIB_TIMING_PARAMETERS)) == 0)
                {
                    mmhiMibStatus |= MMHI_STATUS_MASK_TIMING_PARAMS_MIB_Msk;
                }
                else
                {
                    mmhiMibStatus &= ~MMHI_STATUS_MASK_TIMING_PARAMS_MIB_Msk;
                }

                if (indEnable == true)
                {
                    /* Update Write Indication flag */
                    mmhiMibWriteIndex = MIB_MIB_ID_TIMING;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }
        
        default:
            result = MMHI_ERROR_WPV;

    }

    if ((result == MMHI_SUCCESS) && (mmhiMibWriteIndex > 0))
    {
        /* Update Write Ind parameters */
        (void)memcpy(mmhiMibWriteData.dataValue, pData->dataValue, reqLength);
        mmhiMibWriteData.dataLength = reqLength;
    }

    return result;
}

void MMHI_MIB_WriteIndCallbackRegister(MMHI_MIB_WRITE_IND_CALLBACK callback)
{
    mmhiMibCallback = callback;
}

void MMHI_MIB_Tasks(void)
{
    if (mmhiMibWriteIndex > 0)
    {
        mmhiMibWriteIndex = 0;
        if (mmhiMibCallback != NULL)
        {
            mmhiMibCallback(mmhiMibWriteIndex, &mmhiMibWriteData);
        }
    }
}

/*******************************************************************************
 End of File
*/