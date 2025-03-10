/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal.c

  Summary:
    Platform Abstraction Layer (PAL) Interface source file.

  Description:
    Platform Abstraction Layer (PAL) Interface source file. The PAL
    module provides a simple interface to manage the M&M PHY layer.
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

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"
#include "service/pcoup/srv_pcoup.h"
#include "pal_local.h"
#include "pal.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

static PAL_DATA palData = {0};

// *****************************************************************************
// *****************************************************************************
// Section: Local Callbacks
// *****************************************************************************
// *****************************************************************************

static void lPAL_ExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    if (exceptionObj == DRV_PLC_PHY_EXCEPTION_UNEXPECTED_KEY)
    {
        palData.statsErrorUnexpectedKey++;
    }

    if (exceptionObj == DRV_PLC_PHY_EXCEPTION_RESET)
    {
        palData.statsErrorReset++;
    }

    if (exceptionObj == DRV_PLC_PHY_EXCEPTION_CRITICAL_ERROR)
    {
        palData.statsErrorCritical++;
    }

    if (exceptionObj == DRV_PLC_PHY_EXCEPTION_DEBUG)
    {
        palData.statsErrorDebug++;
    }

    /* Set Error Status */
    palData.status = PAL_STATUS_ERROR;
    palData.state = PAL_STATE_ERROR;
}

static void lPAL_DataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context)
{
    PAL_RESULT result = PAL_RESULT_ERROR;

    /* Avoid warning */
    (void)context;

    palData.waitingTxCfm = false;

    /* Handle result of transmission and transfer it to upper layer */
    switch(cfmObj->result)
    {
        case DRV_PLC_PHY_TX_RESULT_SUCCESS:
            result = PAL_RESULT_SUCCESS;
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_LENGTH:
            result = PAL_RESULT_INVALID_PARAMETER;
            break;
        case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
        case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
            result = PAL_RESULT_BUSY_CH;
            break;
        case DRV_PLC_PHY_TX_RESULT_BUSY_TX:
        case DRV_PLC_PHY_TX_RESULT_PROCESS:
        case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_120:
        case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_110:
            result = PAL_RESULT_DENIED;
            break;
        case DRV_PLC_PHY_TX_RESULT_TIMEOUT:
            result = PAL_RESULT_TIMEOUT;
            break;
        case DRV_PLC_PHY_TX_CANCELLED:
        case DRV_PLC_PHY_TX_RESULT_NO_TX:
            result = PAL_RESULT_ERROR;
            break;
        default:
            result = PAL_RESULT_ERROR;
            break;
    }

    /* Send Confirm to upper layer */
    if (palData.initHandlers.palTxConfirm != NULL)
    {
        palData.initHandlers.palTxConfirm(result);
    }

}

static void lPAL_DataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context)
{
    PAL_RX_PARAMS rxParams;
    DRV_PLC_PHY_PIB_OBJ pibObj;
    uint8_t enable = 1;

    /* Avoid warning */
    (void)context;

    if (indObj->crcOk == 1U)
    {
        /* Correct CRC */
        rxParams.frameDuration = indObj->frameDuration;
        rxParams.snrHeader = indObj->snrHeader;
        rxParams.snrPayload = indObj->snrPayload;
        rxParams.nbRx = indObj->nbRx;
        rxParams.lqi = indObj->lqi;
        rxParams.rssi = indObj->rssi;

        /* Send Parameters Indication to upper layer */
        if (palData.initHandlers.palRxParamsIndication != NULL)
        {
            palData.initHandlers.palRxParamsIndication(&rxParams);
        }

        /* Send Indication to upper layer */
        if (palData.initHandlers.palDataIndication != NULL)
        {
            palData.initHandlers.palDataIndication(indObj->pReceivedData, indObj->dataLength);
        }

    }
    else if (indObj->crcOk == 0xFEU)
    {
        /* Timeout Error */
    }
    else if (indObj->crcOk == 0xFFU)
    {
        /* CRC Capability Disabled */
        /* Enable it */
        pibObj.id = PLC_ID_CRC_TX_RX_CAPABILITY;
        pibObj.length = 1;
        pibObj.pData = &enable;

        (void) DRV_PLC_PHY_PIBSet(palData.drvHandle, &pibObj);
    }
    else
    {
        /* CRC Error */
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Interface Function Definitions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 11.3 deviated twice. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */

void PAL_Init(PAL_INIT *init)
{
    /* Store callback handlers */
    palData.initHandlers = init->palHandlers;

    /* Clear exceptions statistics */
    palData.statsErrorUnexpectedKey = 0;
    palData.statsErrorReset = 0;
    palData.statsErrorDebug = 0;
    palData.statsErrorCritical = 0;
    palData.waitingTxCfm = false;

    /* Set Status and State */
    palData.status = PAL_STATUS_BUSY;
    palData.state = PAL_STATE_INIT;
}

/* MISRA C-2012 deviation block end */

PAL_STATUS PAL_Status(void)
{
    return palData.status;
}

void PAL_TxRequest(uint8_t *pData, uint16_t length, uint8_t nbFrame, uint32_t delay)
{
    PAL_RESULT result = PAL_RESULT_SUCCESS;
    DRV_PLC_PHY_TRANSMISSION_OBJ txObj;

    if (palData.status != PAL_STATUS_READY)
    {
        result = PAL_RESULT_DENIED;
    }

    if (palData.state != PAL_STATE_IDLE)
    {
        result = PAL_RESULT_DENIED;
    }

    if (palData.waitingTxCfm)
    {
        result = PAL_RESULT_DENIED;
    }

    if (result == PAL_RESULT_SUCCESS)
    {
        palData.waitingTxCfm = true;
        /* Set Transmission Mode */
        txObj.mode = TX_MODE_RELATIVE;
        txObj.timeIni = delay;
        txObj.pTransmitData = pData;
        txObj.dataLength = length;
        txObj.attenuation = 0;
        txObj.nbFrame = nbFrame;
        /* Send frame */
        DRV_PLC_PHY_TxRequest(palData.drvHandle, &txObj);
    }
    else
    {
        palData.waitingTxCfm = false;
        if (palData.initHandlers.palTxConfirm != NULL)
        {
            palData.initHandlers.palTxConfirm(result);
        }

    }
}

void PAL_Reset(void)
{
    /* Clear wait confirm flag */
    palData.waitingTxCfm = false;

    /* Close Driver */
    DRV_PLC_PHY_Close(palData.drvHandle);

    /* Set Status and State to reopen Driver */
    palData.status = PAL_STATUS_BUSY;
    palData.state = PAL_STATE_INIT;
}

PAL_RESULT PAL_GetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj)
{
    if (palData.status != PAL_STATUS_READY)
    {
        /* Ignore request */
        return PAL_RESULT_DENIED;
    }

    /* Get IB */
    if (DRV_PLC_PHY_PIBGet(palData.drvHandle, pibObj))
    {
        return PAL_RESULT_SUCCESS;
    }
    else
    {
        return PAL_RESULT_ERROR;
    }
}

PAL_RESULT PAL_SetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj)
{
    if (palData.status != PAL_STATUS_READY)
    {
        /* Ignore request */
        return PAL_RESULT_DENIED;
    }

    /* Set IB */
    if (DRV_PLC_PHY_PIBSet(palData.drvHandle, pibObj))
    {
        return PAL_RESULT_SUCCESS;
    }
    else
    {
        return PAL_RESULT_ERROR;
    }
}

void PAL_Tasks(void)
{
    switch (palData.state)
    {
        case PAL_STATE_IDLE:
        {
            /* Nothing to do */
            break;
        }

        case PAL_STATE_INIT:
        {
            /* Open PHY Driver */
            palData.drvHandle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, NULL);

            if (palData.drvHandle != DRV_HANDLE_INVALID)
            {
                palData.state = PAL_STATE_OPEN;
            }
            else
            {
                palData.state = PAL_STATE_ERROR;
            }
            break;
        }

        case PAL_STATE_OPEN:
        {
            /* Check Transceiver status */
            if (DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX) == SYS_STATUS_READY)
            {
                /* Configure callbacks */
                DRV_PLC_PHY_ExceptionCallbackRegister(palData.drvHandle, lPAL_ExceptionCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_TxCfmCallbackRegister(palData.drvHandle, lPAL_DataCfmCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_DataIndCallbackRegister(palData.drvHandle, lPAL_DataIndCb, DRV_PLC_PHY_INDEX);

                /* Apply coupling configuration */
                (void) SRV_PCOUP_Set_Config(palData.drvHandle);

                /* Enable TX Enable at the beginning */
                DRV_PLC_PHY_EnableTX(palData.drvHandle, true);
                palData.pvddMonTxEnable = true;
                /* Go to Wait Pvdd state */
                /* Even if PVdd not used, conditions will set following state correctly */
                palData.state = PAL_STATE_WAIT_PVDD_MON;
            }
            break;
        }

        case PAL_STATE_WAIT_PVDD_MON:
        {
            /* Check flag before setting Ready status and Idle state */
            if (palData.pvddMonTxEnable)
            {
                palData.status = PAL_STATUS_READY;
                palData.state = PAL_STATE_IDLE;
            }
            break;
        }

        case PAL_STATE_ERROR:
        {
            /* Reset PAL to reinitialize Driver */
            PAL_Reset();
            break;
        }

        default:
        {
            /* Unknown state, go to Idle */
            palData.state = PAL_STATE_IDLE;
            break;
        }
    }
}

