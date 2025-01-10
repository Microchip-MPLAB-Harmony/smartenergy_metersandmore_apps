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
#include "mmhi.h"
#include "mmhi_mib.h"

#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

#define MMHI_MIB_MAC_CONFIG_INVALID_VALUES_MASK   0xFEU

/* MM Host Interface Data */
static MMHI_MIB_DB mmhiMibData;
static MMHI_MIB_DB mmhiMibDefaultData = {
    {MIB_RX_NORMAL, MIB_TX_NORMAL},
    {0xAAA5, 0, 0, 0, 0, MMHI_MIB_FW_VERSION_MAJOR, MMHI_MIB_FW_VERSION_MINOR},
    {{0}, {{0}}},
    {{0}, 0, 0, 3},
    {{0}, {0}},
    {0},
    {{0}},
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
    (void)memcpy(mibData, &mmhiMibDefaultData, sizeof(MMHI_MIB_DB));
}

static MMHI_RESULT setEncryptionKeys(uint8_t *paramsBuf)
{
    MMHI_RESULT result = MMHI_ERROR;
    uint8_t *writeKey;
    uint8_t *readKey;
    AL_RESULT alResult;
    AL_IB_VALUE alValue;

    /* Set pointers */
    writeKey = paramsBuf;
    readKey = (paramsBuf + 16);

    /* Set IBs */
    alValue.length = AL_KEY_LENGTH;
    (void)memcpy(alValue.value, writeKey, AL_KEY_LENGTH);
    alResult = AL_SetRequest(AL_AUTH_WRITE_KEY_K1_IB, 0, (const AL_IB_VALUE *)&alValue);
    if (alResult == AL_SUCCESS)
    {
        (void)memcpy(alValue.value, readKey, AL_KEY_LENGTH);
        alResult = AL_SetRequest(AL_AUTH_READ_KEY_K2_IB, 0, (const AL_IB_VALUE *)&alValue);
        if (alResult == AL_SUCCESS)
        {
            result = MMHI_SUCCESS;
        }
    }

    /* Return global resut of parameters set */
    return result;
}

static MMHI_RESULT setLmon(uint8_t *paramsBuf)
{
    MMHI_RESULT result = MMHI_ERROR;
    AL_RESULT alResult;
    AL_IB_VALUE alValue;

    /* Set IB */
    alValue.length = AL_LMON_LENGTH;
    (void)memcpy(alValue.value, paramsBuf, AL_LMON_LENGTH);
    alResult = AL_SetRequest(AL_AUTH_LMON_IB, 0, (const AL_IB_VALUE *)&alValue);
    if (alResult == AL_SUCCESS)
    {
        result = MMHI_SUCCESS;
    }

    /* Return global resut of parameters set */
    return result;
}

static MMHI_RESULT setTimingParams(uint8_t *paramsBuf)
{
    MMHI_RESULT result = MMHI_ERROR;
    uint16_t tEl16, tDelay16, tSlot16;
    uint32_t tEl32, tDelay32, tSlot32;
    uint8_t tct;
    AL_RESULT alResult;
    AL_IB_VALUE alValue;

    /* Get 16-bit values from buffer */
    tEl16 = ((uint16_t)paramsBuf[1] << 8) + paramsBuf[0];
    tDelay16 = ((uint16_t)paramsBuf[3] << 8) + paramsBuf[2];
    tSlot16 = ((uint16_t)paramsBuf[7] << 8) + paramsBuf[6];

    /* Convert to microseconds to set on lower layers */
    tEl32 = (uint32_t)tEl16 * 1000UL;
    tDelay32 = (uint32_t)tDelay16 * 1000UL;
    tSlot32 = (uint32_t)tSlot16 * 1000UL;

    /* Get TCT */
    tct = paramsBuf[8];

    /* Set IBs */
    alValue.length = 4U;
    (void)memcpy(alValue.value, (uint8_t *)&tEl32, MAC_ADDRESS_SIZE);
    alResult = AL_SetRequest(AL_MAC_TIME_ELABORATION_US_IB, 0, (const AL_IB_VALUE *)&alValue);
    if (alResult == AL_SUCCESS)
    {
        (void)memcpy(alValue.value, (uint8_t *)&tDelay32, MAC_ADDRESS_SIZE);
        alResult = AL_SetRequest(AL_MAC_ADDITIONAL_DELAY_US_IB, 0, (const AL_IB_VALUE *)&alValue);
        if (alResult == AL_SUCCESS)
        {
            (void)memcpy(alValue.value, (uint8_t *)&tSlot32, MAC_ADDRESS_SIZE);
            alResult = AL_SetRequest(AL_MAC_TIME_SLOT_US_IB, 0, (const AL_IB_VALUE *)&alValue);
        }
        if (alResult == AL_SUCCESS)
        {
            alValue.length = 1U;
            alValue.value[0] = tct;
            alResult = AL_SetRequest(AL_NM_TCT_IB, 0, (const AL_IB_VALUE *)&alValue);
            if (alResult == AL_SUCCESS)
            {
                result = MMHI_SUCCESS;
            }
        }
    }

    /* Return global resut of parameters set */
    return result;
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
    mmhiMibWriteIndex = (MMHI_MIB_INDEX)0U;
    lMMHI_MIB_GetDefault(&mmhiMibData);
    mmhiMibStatus = HI_SMSK_MAC_CFG_MIB_Msk |
            HI_SMSK_FW_RELEASE_MIB_Msk |
            HI_SMSK_MNF_DATA_MIB_Msk |
            HI_SMSK_SCA_MIB_Msk |
            HI_SMSK_TIMING_PARAMS_MIB_Msk;
}

uint16_t MMHI_MIB_GetStatus(void)
{
    return mmhiMibStatus;
}

MMHI_RESULT MMHI_MIB_Get(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData)
{
    MMHI_RESULT result = MMHI_SUCCESS;
    uint32_t size32;

    switch (mibIndex)
    {
        case MIB_MIB_ID_MAC_CONFIG:
            size32 = sizeof(MMHI_MIB_MAC_CONFIG);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.macConfig, pData->dataLength);
            break;

        case MIB_MIB_ID_FW_RELEASE:
            size32 = sizeof(MMHI_MIB_FW_VERSION);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.fwVersion, pData->dataLength);
            break;

        case MIB_MIB_ID_MANUF_DATA:
            size32 = sizeof(MMHI_MIB_MANUFACTURER_DATA);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.manufacturer, pData->dataLength);
            break;

        case MIB_MIB_ID_LOGICAL_ADDRESS:
            size32 = sizeof(MMHI_MIB_LOGICAL_ADDRESS);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.address, pData->dataLength);
            break;

        case MIB_MIB_ID_SECURITY_FLAGS:
            size32 = sizeof(MMHI_MIB_SECURITY_FLAGS);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.securityFlags, pData->dataLength);
            break;

        case MIB_MIB_ID_LMON:
            size32 = sizeof(MMHI_MIB_LMON);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.securityLmon, pData->dataLength);
            break;

        case MIB_MIB_ID_TIMING:
            size32 = sizeof(MMHI_MIB_TIMING_PARAMETERS);
            pData->dataLength = (uint8_t)size32;
            (void)memcpy(pData->dataValue, (uint8_t *)&mmhiMibData.timing, pData->dataLength);
            break;

        case MIB_MIB_ID_ENCRYPTION_KEYS: /* Write Only */
        default:
            result = MMHI_ERROR_WPV;
            break;

    }

    return result;
}

MMHI_RESULT MMHI_MIB_Set(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData, bool indEnable)
{
    MMHI_RESULT result = MMHI_SUCCESS;
    uint8_t reqLength;
    AL_RESULT alResult;
    AL_IB_VALUE alValue;
    uint8_t *currentVal;
    uint8_t *defaultVal;

    reqLength = pData->dataLength;

    switch (mibIndex)
    {
        case MIB_MIB_ID_MAC_CONFIG:
        {
            if (reqLength == sizeof(MMHI_MIB_MAC_CONFIG))
            {
                if (((pData->dataValue[0] & MMHI_MIB_MAC_CONFIG_INVALID_VALUES_MASK) != 0U) ||
                    ((pData->dataValue[1] & MMHI_MIB_MAC_CONFIG_INVALID_VALUES_MASK) != 0U))
                {
                    /* Invalid value */
                    result = MMHI_ERROR_WPV;
                }
                else
                {
                    /* Update Status flags */
                    (void)memcpy((uint8_t *)&mmhiMibData.macConfig, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.macConfig;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.macConfig;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_MAC_CONFIG)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_MAC_CFG_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_MAC_CFG_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_MAC_CONFIG;
                    }
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
                /* Set value on lower layers */
                alValue.length = MAC_ADDRESS_SIZE;
                /* ACA is already reversed according to HI Spec */
                (void)memcpy(alValue.value, &pData->dataValue[16], MAC_ADDRESS_SIZE);
                alResult = AL_SetRequest(AL_MAC_ACA_ADDRESS_IB, 0, (const AL_IB_VALUE *)&alValue);

                /* If set correct on lower layer, set on HI */
                if (alResult == AL_SUCCESS)
                {
                    (void)memcpy((uint8_t *)&mmhiMibData.manufacturer, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.manufacturer;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.manufacturer;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_MANUFACTURER_DATA)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_MNF_DATA_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_MNF_DATA_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_MANUF_DATA;
                    }
                }
                else
                {
                    result = MMHI_ERROR_BUSY;
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
                /* Set value on lower layers */
                alValue.length = MAC_ADDRESS_SIZE;
                (void)memcpy(alValue.value, pData->dataValue, MAC_ADDRESS_SIZE);
                alResult = AL_SetRequest(AL_MAC_SCA_ADDRESS_IB, 0, (const AL_IB_VALUE *)&alValue);

                /* If set correct on lower layer, set on HI */
                if (alResult == AL_SUCCESS)
                {
                    (void)memcpy((uint8_t *)&mmhiMibData.address, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.address;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.address;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_LOGICAL_ADDRESS)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_SCA_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_SCA_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_LOGICAL_ADDRESS;
                    }
                }
                else
                {
                    result = MMHI_ERROR_BUSY;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_ENCRYPTION_KEYS:
        {
            if (reqLength == sizeof(MMHI_MIB_ENCRYPTION_KEYS))
            {
                /* Set value on lower layers */
                /* If set correct on lower layer, set on HI */
                if (setEncryptionKeys(pData->dataValue) == MMHI_SUCCESS)
                {
                    (void)memcpy((uint8_t *)&mmhiMibData.securityKeys, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.securityKeys;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.securityKeys;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_ENCRYPTION_KEYS)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_ENCRYPTION_KEYS_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_ENCRYPTION_KEYS_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_ENCRYPTION_KEYS;
                    }
                }
                else
                {
                    result = MMHI_ERROR_BUSY;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_SECURITY_FLAGS:
        {
            if (reqLength == sizeof(MMHI_MIB_SECURITY_FLAGS))
            {
                /* Set value */
                (void)memcpy((uint8_t *)&mmhiMibData.securityFlags, pData->dataValue, reqLength);
                currentVal = (uint8_t *)&mmhiMibData.securityFlags;
                defaultVal = (uint8_t *)&mmhiMibDefaultData.securityFlags;
                if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_SECURITY_FLAGS)) == 0)
                {
                    mmhiMibStatus |= HI_SMSK_SECURITY_FLAGS_MIB_Msk;
                }
                else
                {
                    mmhiMibStatus &= ~HI_SMSK_SECURITY_FLAGS_MIB_Msk;
                }

                if (indEnable == true)
                {
                    /* Update Write Indication flag */
                    mmhiMibWriteIndex = MIB_MIB_ID_SECURITY_FLAGS;
                }
            }
            else
            {
                result = MMHI_ERROR_WPL;
            }
            break;
        }

        case MIB_MIB_ID_LMON:
        {
            if (reqLength == sizeof(MMHI_MIB_LMON))
            {
                /* Set value on lower layers */
                /* If set correct on lower layer, set on HI */
                if (setLmon(pData->dataValue) == MMHI_SUCCESS)
                {
                    (void)memcpy((uint8_t *)&mmhiMibData.securityLmon, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.securityLmon;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.securityLmon;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_LMON)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_LMON_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_LMON_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_LMON;
                    }
                }
                else
                {
                    result = MMHI_ERROR_BUSY;
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
                /* Set value on lower layers */
                /* If set correct on lower layer, set on HI */
                if (setTimingParams(pData->dataValue) == MMHI_SUCCESS)
                {
                    (void)memcpy((uint8_t *)&mmhiMibData.timing, pData->dataValue, reqLength);
                    currentVal = (uint8_t *)&mmhiMibData.timing;
                    defaultVal = (uint8_t *)&mmhiMibDefaultData.timing;
                    if (memcmp(currentVal, defaultVal, sizeof(MMHI_MIB_TIMING_PARAMETERS)) == 0)
                    {
                        mmhiMibStatus |= HI_SMSK_TIMING_PARAMS_MIB_Msk;
                    }
                    else
                    {
                        mmhiMibStatus &= ~HI_SMSK_TIMING_PARAMS_MIB_Msk;
                    }

                    if (indEnable == true)
                    {
                        /* Update Write Indication flag */
                        mmhiMibWriteIndex = MIB_MIB_ID_TIMING;
                    }
                }
                else
                {
                    result = MMHI_ERROR_BUSY;
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
            break;

    }

    if ((result == MMHI_SUCCESS) && ((uint8_t)mmhiMibWriteIndex > 0U))
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
    AL_RESULT alResult;
    AL_IB_VALUE alValue;
    uint32_t size32;

    /* Check whether there is a change in any of IB requiring indication */
    if (mmhiMibData.timing.tct != 0U)
    {
        alResult = AL_GetRequest(AL_NM_TCT_IB, 0, &alValue);
        if (alResult == AL_SUCCESS)
        {
            /* Compare values */
            if (alValue.value[0] != mmhiMibData.timing.tct)
            {
                mmhiMibData.timing.tct = alValue.value[0];
                mmhiMibWriteIndex = MIB_MIB_ID_TIMING;
                size32 = sizeof(MMHI_MIB_TIMING_PARAMETERS);
                mmhiMibWriteData.dataLength = (uint8_t)size32;
                (void)memcpy(mmhiMibWriteData.dataValue, (uint8_t *)&mmhiMibData.timing, mmhiMibWriteData.dataLength);
            }
        }
    }

    if ((uint8_t)mmhiMibWriteIndex > 0U)
    {
        if (mmhiMibCallback != NULL)
        {
            mmhiMibCallback(mmhiMibWriteIndex, &mmhiMibWriteData);
        }
        mmhiMibWriteIndex = (MMHI_MIB_INDEX)0U;
    }
}

/*******************************************************************************
 End of File
*/