/*******************************************************************************
  Meters and More Host Interface Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    mmhi.c

  Summary:
    Meters and More Host Interface interface implementation.

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

#include "service/pcrc/srv_pcrc.h"
#include "stack/metersandmore/mmhi/mmhi.h"
#include "stack/metersandmore/mmhi/mmhi_definitions.h"
#include "stack/metersandmore/mmhi/mmhi_local.h"
#include "stack/metersandmore/mmhi/mmhi_mib.h"
#include "system/time/sys_time.h"
#include "configuration.h"
#include "osal/osal.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
/* Custom Command Data */
#define MMHI_INVALID_CUSTOM_COMMAND   0xFFU
#define MMHI_MAX_CUSTOM_COMMANDS      (5U)
static MMHI_CUSTOM_CMD_DATA sMmhiCustomCmdData[MMHI_MAX_CUSTOM_COMMANDS];

/* MM Host Interface Data */
static MMHI_DATA mmhiData = {0};

static uint8_t mmhiTxBuffer[MMHI_FRAME_MAX_LENGTH];
static uint8_t mmhiRxBuffer[MMHI_FRAME_MAX_LENGTH];

static uint8_t mmhiDsapByProtocol[] = {0x00, 0x00, 0x02, 0xFF, 0x03, 0xFF, 0x01, 0xFF};

static void lMMHI_ClearReceptionState(void);

// *****************************************************************************
// *****************************************************************************
// Section: Local Callbacks
// *****************************************************************************
// *****************************************************************************
static void lMMHI_TicTimerCallback ( uintptr_t context )
{
    int32_t diff;

    mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
    diff = mmhiData.pReceiveData - mmhiRxBuffer;
    mmhiData.rcvFrameLength = (uint8_t)diff;
    mmhiData.pReceiveData = mmhiRxBuffer;
}

static void lMMHI_TackTimerCallback ( uintptr_t context )
{
    mmhiData.tackTimer = SYS_TIME_HANDLE_INVALID;
    /* Retry last Command Frame: Not ACKed */
    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMDRET_Msk;

    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Tack!!!\r\n");
}

static void lMMHI_TsrTimerCallback ( uintptr_t context )
{
    mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
    /* Finish reception state */
    lMMHI_ClearReceptionState();

    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Tsr!!!\r\n");
}

static void lMMHI_TxDelayTimerCallback ( uintptr_t context )
{
    mmhiData.txDelayTimer = SYS_TIME_HANDLE_INVALID;

    if ((mmhiData.swReset == true) && (mmhiData.rstConfirmSent == true))
    {
        RSTC_Reset(RSTC_ALL_RESET);
    }

    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "TxDelay!!!\r\n");
}

static void lMMHI_USART_ReadCallback( uintptr_t context )
{
    if(MMHI_USART_ERROR_NONE == mmhiData.uartPLIB->errorGet())
    {
        if (mmhiData.state == MMHI_STATE_RECEIVING)
        {
            uint16_t totalRcvLength;
            int32_t rcvLen;
            uintptr_t ctx = 0;

            *mmhiData.pReceiveData++ = mmhiData.rcvByte;
            (void) mmhiData.uartPLIB->readFn(&mmhiData.rcvByte, 1U);

            /* Cancel TSR timer */
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                (void) SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
                mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
            }

            // Transfer completed successfully
            mmhiData.statusMessage.status |= MMHI_STATUS_RX_Msk;
            rcvLen = mmhiData.pReceiveData - mmhiRxBuffer;
            totalRcvLength = (uint16_t)rcvLen;

            if (totalRcvLength == (MMHI_FRAME_MAX_LENGTH - 1U))
            {
                mmhiData.rcvFrameLength = (uint8_t)totalRcvLength;
            }
            else
            {
                /* Launch TIC timer */
                if (mmhiData.ticTimer != SYS_TIME_HANDLE_INVALID)
                {
                    (void) SYS_TIME_TimerDestroy(mmhiData.ticTimer);
                    mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
                }

                mmhiData.ticTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TicTimerCallback, ctx,
                            MMHI_PROTOCOL_TIMEOUT_TIC_MS, SYS_TIME_SINGLE);
            }
        }
    }
}

static void lMMHI_USART_WriteCallback( uintptr_t context )
{
    uint8_t mask;
    uintptr_t ctx = 0;

    mask = (uint8_t)context;
    mmhiData.startFrameTxMask &= ~mask;
    mmhiData.statusMessage.status &= ~MMHI_STATUS_TX_Msk;

    if (mask == MMHI_FRAME_START_MASK_STATUS_Msk)
    {
        /* Start reception and launch TSR timer */
        mmhiData.state = MMHI_STATE_RECEIVING;
        mmhiData.pReceiveData = mmhiRxBuffer;
        (void) mmhiData.uartPLIB->readFn(&mmhiData.rcvByte, 1U);
        if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
        {
            (void) SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
        }
        mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback, ctx,
                MMHI_PROTOCOL_TIMEOUT_TSR_MS, SYS_TIME_SINGLE);
    }
}

static void lMMHI_ExternalInterruptRTSHandler( PIO_PIN pin, uintptr_t context )
{
    /* Send Status message */
    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_STATUS_Msk;
    /* Restart NACK counter */
    mmhiData.nackCounter = 0;
}

static void lMMHI_WriteIndicationCallback( MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pMibData )
{
    uint8_t *pData = mmhiTxBuffer;

    *pData++ = (uint8_t)MMHI_FS_COMMAND;
    *pData++ = pMibData->dataLength;
    *pData++ = (uint8_t)MMHI_CMD_MIB_WRITE_IND;
    *pData++ = (uint8_t)mibIndex;
    (void) memcpy(pData, pMibData->dataValue, pMibData->dataLength);

    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;

    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> WRITE_IND\r\n");
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

static void lMMHI_ClearReceptionState(void)
{
    /* Finish reception state */
    mmhiData.state = MMHI_STATE_IDLE;
    mmhiData.pReceiveData = mmhiRxBuffer;
    mmhiData.rcvFrameLength = 0;
    mmhiData.statusMessage.status &= ~MMHI_STATUS_RX_Msk;
    mmhiData.nackCounter = 0;

    if (mmhiData.ticTimer != SYS_TIME_HANDLE_INVALID)
    {
        (void) SYS_TIME_TimerDestroy(mmhiData.ticTimer);
        mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
    }
    if (mmhiData.tackTimer != SYS_TIME_HANDLE_INVALID)
    {
        (void) SYS_TIME_TimerDestroy(mmhiData.tackTimer);
        mmhiData.tackTimer = SYS_TIME_HANDLE_INVALID;
    }
    if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
    {
        (void) SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
        mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
    }
}

static uint8_t lMMHI_CheckCustomCommand(MMHI_COMMAND command)
{
    MMHI_CUSTOM_CMD_DATA* pCmdTable = sMmhiCustomCmdData;
    uint8_t index;

    for (index = 0; index < MMHI_MAX_CUSTOM_COMMANDS; index++)
    {
        if ((uint8_t)command == pCmdTable->commandCode)
        {
            return index;
        }

        pCmdTable++;
    }

    return MMHI_INVALID_CUSTOM_COMMAND;
}

static bool lMMHI_ProcessCustomFrame(void)
{
    uint8_t cmdIndex;
    bool result = false;

    cmdIndex = lMMHI_CheckCustomCommand(mmhiData.rcvFrameData.commandCode);
    if (cmdIndex != MMHI_INVALID_CUSTOM_COMMAND)
    {
        MMHI_CMD_FRAME *pRcvFrameData;

        result = true;

        pRcvFrameData = &mmhiData.rcvFrameData;
        sMmhiCustomCmdData[cmdIndex].callback(pRcvFrameData->pPayload, pRcvFrameData->length);
    }

    return result;
}

static bool lMMHI_ProcessRcvCommand(void)
{
    MMHI_CMD_FRAME *pRcvFrameData;
    bool resultRet = true;

    pRcvFrameData = &mmhiData.rcvFrameData;

    if (pRcvFrameData->commandCode == MMHI_CMD_BIO_RESET_REQ)
    {
        uint8_t *pData = mmhiTxBuffer;

        *pData++ = (uint8_t)MMHI_FS_COMMAND;
        *pData++ = 0;
        *pData++ = (uint8_t)MMHI_CMD_BIO_RESET_CFM;
        *pData++ = 0;

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;

        /* Set reset flags to be performed when TX is completed */
        mmhiData.swReset = true;
        mmhiData.rstConfirmSent = false;

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- BIO_RESET_REQ\r\n");
    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_MIB_WRITE_REQ)
    {
        MMHI_MIB_DATA *pMibData = &mmhiData.mibData;
        uint8_t *pData = mmhiTxBuffer;
        MMHI_MIB_INDEX mibIndex;
        MMHI_RESULT result;
        uint8_t mibLength;

        mibLength = pRcvFrameData->length;
        if (mibLength > MMHI_MIB_MAX_LENGTH_DATA)
        {
            *pData++ = (uint8_t)MMHI_FS_COMMAND;
            *pData++ = 0;
            *pData++ = (uint8_t)MMHI_CMD_MIB_WRITE_NCFM;
            *pData++ = (uint8_t)MMHI_ERROR_WPL;
        }
        else
        {
            mibIndex = (MMHI_MIB_INDEX)pRcvFrameData->pPayload[0];
            (void) memcpy(pMibData->dataValue, &pRcvFrameData->pPayload[1], mibLength);
            pMibData->dataLength = mibLength;

            result = MMHI_MIB_Set(mibIndex, pMibData, false);

            if (result == MMHI_SUCCESS)
            {
                *pData++ = (uint8_t)MMHI_FS_COMMAND;
                *pData++ = 0;
                *pData++ = (uint8_t)MMHI_CMD_MIB_WRITE_CFM;
                *pData++ = (uint8_t)mibIndex;

                mmhiData.statusMessage.mibStatusMask = MMHI_MIB_GetStatus();
            }
            else
            {
                *pData++ = (uint8_t)MMHI_FS_COMMAND;
                *pData++ = 0;
                *pData++ = (uint8_t)MMHI_CMD_MIB_WRITE_NCFM;
                *pData++ = (uint8_t)result;
            }
        }

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- MIB_WRITE_REQ\r\n");
    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_MIB_READ_REQ)
    {
        MMHI_MIB_DATA *pMibData = &mmhiData.mibData;
        uint8_t *pData = mmhiTxBuffer;
        MMHI_MIB_INDEX mibIndex;
        MMHI_RESULT result;

        mibIndex = (MMHI_MIB_INDEX)pRcvFrameData->pPayload[0];

        result = MMHI_MIB_Get(mibIndex, pMibData);

        if (result == MMHI_SUCCESS)
        {
            *pData++ = (uint8_t)MMHI_FS_COMMAND;
            *pData++ = pMibData->dataLength - 1U;
            *pData++ = (uint8_t)MMHI_CMD_MIB_READ_CFM;
            (void) memcpy(pData, pMibData->dataValue, pMibData->dataLength);
        }
        else
        {
            *pData++ = (uint8_t)MMHI_FS_COMMAND;
            *pData++ = 0;
            *pData++ = (uint8_t)MMHI_CMD_MIB_READ_NCFM;
            *pData++ = (uint8_t)result;
        }

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- MIB_READ_REQ\r\n");

    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_SLAVE_DATA_REQ)
    {
        uint8_t *pData = pRcvFrameData->pPayload;
        uint8_t protocol;
        uint8_t reqId;
        uint8_t dsap;
        uint8_t wpvError = 0x03; /* WPV Error Code */

        protocol = *pData++;
        reqId = *pData++;
        dsap = mmhiDsapByProtocol[(protocol & 0x07U)];

        if (dsap <= 0x02U)
        {
            if (mmhiData.macDataCallback != NULL)
            {
                mmhiData.macDataCallback(dsap, reqId, pData,
                        pRcvFrameData->length - 1U);
            }

            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- SLAVE_DATA_REQ\r\n");
        }
        else
        {
            /* Invalid protocol. Generate negative Confirm */
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- Wrong SLAVE_DATA_REQ. Send Negative Confirm\r\n");
            (void) MMHI_SendCommandFrame((uint8_t)MMHI_CMD_SLAVE_DATA_NCFM, &wpvError, 1);
        }

    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_MASTER_DATA_REQ)
    {
        uint8_t *pData = pRcvFrameData->pPayload;
        uint8_t protocol;
        uint8_t reqId;
        uint8_t dsap;
        uint8_t wpvError = 0x03; /* WPV Error Code */

        protocol = *pData++;
        reqId = *pData++;
        dsap = mmhiDsapByProtocol[(protocol & 0x07U)];

        if (dsap <= 0x02U)
        {
            if (mmhiData.macDataCallback != NULL)
            {
                mmhiData.macDataCallback(dsap, reqId, pData,
                        pRcvFrameData->length - 1U);
            }

            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- MASTER_DATA_REQ\r\n");
        }
        else
        {
            /* Invalid protocol. Generate negative Confirm */
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- Wrong MASTER_DATA_REQ. Send Negative Confirm\r\n");
            (void) MMHI_SendCommandFrame((uint8_t)MMHI_CMD_MASTER_DATA_NCFM, &wpvError, 1);
        }

    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_HI_PING_REQ)
    {
        uint8_t *pData = mmhiTxBuffer;
        uint8_t copyLen;

        *pData++ = (uint8_t)MMHI_FS_COMMAND;
        *pData++ = pRcvFrameData->length;
        *pData++ = (uint8_t)MMHI_CMD_HI_PING_CFM;
        copyLen = pRcvFrameData->length + 1U;
        (void) memcpy(pData, pRcvFrameData->pPayload, copyLen);

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- HI_PING_REQ\r\n");
    }
    else
    {
        resultRet = false;
    }

    return resultRet;
}

static bool lMMHI_ProcessRcvFrame(void)
{
    bool result = true;
    uintptr_t ctx = 0;

    /* Check frame type */
    switch ( mmhiData.rcvFrameData.frameStart )
    {
        case MMHI_FS_COMMAND:
        case MMHI_FS_COMMAND_RETRY:
        {
            if (lMMHI_ProcessRcvCommand() == true)
            {
                // Send ACK
                mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_ACK_Msk;
            }
            else
            {
                result = false;
            }
        }
        break;

        case MMHI_FS_STATUS:
        {
           // TBD
        }
        break;

        case MMHI_FS_ACK:
        {
            /* Stop TACK timer */
            if (mmhiData.tackTimer != SYS_TIME_HANDLE_INVALID)
            {
                (void) SYS_TIME_TimerDestroy(mmhiData.tackTimer);
            }

            /* Start timer to finish reception state */
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                (void) SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
            }
            mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback,
                ctx, MMHI_PROTOCOL_TIMEOUT_TSR_MS, SYS_TIME_SINGLE);

            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- ACK\r\n");

        }
        break;

        case MMHI_FS_NACK:
        {
            if (mmhiData.retryCmd == false)
            {
                /* Retry last command frame */
                mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMDRET_Msk;
            }

            /* Start timer to finish reception state */
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                (void) SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
            }
            mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback,
                ctx, MMHI_PROTOCOL_TIMEOUT_TSR_MS, SYS_TIME_SINGLE);

            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- NACK\r\n");
        }
        break;

        default:
            result = false;
        break;
    }

    return result;
}

static MMHI_RESULT lMMHI_CheckRcvFrame(void)
{
    MMHI_CMD_FRAME *pRcvFrameData;
    uint8_t *pData;
    uint16_t crcCalc;
    uint8_t crcDataLen;
    MMHI_RESULT result = MMHI_SUCCESS;

    pRcvFrameData = &mmhiData.rcvFrameData;

    pRcvFrameData->frameStart = (MMHI_CMD_FRAME_START)mmhiRxBuffer[MMHI_CMD_FRAME_FS_Pos];
    pRcvFrameData->length = mmhiRxBuffer[MMHI_CMD_FRAME_LEN_Pos];
    pRcvFrameData->commandCode = (MMHI_COMMAND)mmhiRxBuffer[MMHI_CMD_FRAME_CMD_Pos];
    pRcvFrameData->pPayload = &mmhiRxBuffer[MMHI_CMD_FRAME_PAYLOAD_Pos];

    if ((pRcvFrameData->frameStart == MMHI_FS_COMMAND) ||
        (pRcvFrameData->frameStart == MMHI_FS_COMMAND_RETRY))
    {
        /* Get CRC16 (Len, Cmd, Payload) */
        crcDataLen = pRcvFrameData->length + 3U;
        crcCalc = (uint16_t)SRV_PCRC_GetValue(&mmhiRxBuffer[MMHI_CMD_FRAME_LEN_Pos],
                    (size_t)crcDataLen, PCRC_HT_GENERIC, PCRC_CRC16, 0);
        pData = pRcvFrameData->pPayload + pRcvFrameData->length + 1;
        pRcvFrameData->checksum = (uint16_t)*pData++;
        pRcvFrameData->checksum += (((uint16_t)*pData) << 8U);

        /* Check CRC16 */
        if (pRcvFrameData->checksum != crcCalc)
        {
            return MMHI_ERROR_CRC;
        }

        /* Check Commands integrity */
        if (pRcvFrameData->commandCode == MMHI_CMD_BIO_RESET_REQ)
        {
            if (pRcvFrameData->length != 0U)
            {
                result = MMHI_ERROR_WPL;
            }
        }
        else if (pRcvFrameData->commandCode == MMHI_CMD_MIB_WRITE_REQ)
        {
            // TBD
        }
        else if (pRcvFrameData->commandCode == MMHI_CMD_MIB_READ_REQ)
        {
            // TBD
        }
        else if (pRcvFrameData->commandCode == MMHI_CMD_SLAVE_DATA_REQ)
        {
            // TBD
        }
        else if (pRcvFrameData->commandCode == MMHI_CMD_MASTER_DATA_REQ)
        {
            // TBD
        }
        else if (pRcvFrameData->commandCode == MMHI_CMD_HI_PING_REQ)
        {
            // TBD
        }
        else
        {
            uint8_t cmdIndex;

            cmdIndex = lMMHI_CheckCustomCommand(mmhiData.rcvFrameData.commandCode);
            if (cmdIndex == (uint8_t)MMHI_INVALID_CUSTOM_COMMAND)
            {
                result = MMHI_ERROR_COMMAND;
            }
        }
    }

    return result;

}

static void lMMHI_sendResetIndication(void)
{
    uint8_t *pData = mmhiTxBuffer;
    uint32_t rstCause;
    MMHI_RESET_CAUSE mmhcRstCause;

    /* Get reset cause */
    rstCause = (RSTC_ResetCauseGet() >> RSTC_SR_RSTTYP_Pos);

    switch (rstCause)
    {
        case RSTC_SR_RSTTYP_WDT0_RST_Val:
            mmhcRstCause = MMHI_RST_WDT;
            break;

        case RSTC_SR_RSTTYP_SOFT_RST_Val:
            mmhcRstCause = MMHI_RST_BIO;
            break;

        case RSTC_SR_RSTTYP_GENERAL_RST_Val:
        case RSTC_SR_RSTTYP_BACKUP_RST_Val:
        case RSTC_SR_RSTTYP_USER_RST_Val:
        case RSTC_SR_RSTTYP_CORE_SM_RST_Val:
        case RSTC_SR_RSTTYP_CPU_FAIL_RST_Val:
        case RSTC_SR_RSTTYP_SLCK_XTAL_RST_Val:
        case RSTC_SR_RSTTYP_WDT1_RST_Val:
        case RSTC_SR_RSTTYP_PORVDD3V3_RST_Val:
        default:
            mmhcRstCause = MMHI_RST_HW;
            break;
    }

    *pData++ = (uint8_t)MMHI_FS_COMMAND;
    *pData++ = 0;
    *pData++ = (uint8_t)MMHI_CMD_BIO_RESET_IND;
    /* All reconfigurable MIB objects has been reconfigured */
    /* Reconfiguration status (Bit 7) */
    *pData++ = (1U << 7) | (uint8_t)mmhcRstCause;

    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
}

// *****************************************************************************
// *****************************************************************************
// Section: System Interface Functions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 Rule 11.3 deviated:1 Deviation record ID -  H3_MISRAC_2012_R_11_3_DR_1 */
/* MISRA C-2012 Rule 11.8 deviated:1 Deviation record ID -  H3_MISRAC_2012_R_11_8_DR_1 */

SYS_MODULE_OBJ MMHI_Initialize(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT* const init
)
{
    const MMHI_INIT* initConfig = (const MMHI_INIT* )init;
    MMHI_CUSTOM_CMD_DATA* pCmdTable = sMmhiCustomCmdData;
    uint8_t cmdTableindex;

    /* Check instance */
    if (index != MMHI_INDEX)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Check previously initialized */
    if (mmhiData.status != MMHI_STATUS_UNINITIALIZED)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    mmhiData.uartPLIB = (MMHI_PLIB_INTERFACE*)initConfig->deviceInitData->uartPLIB;
    mmhiData.statusMessage.frameStart = MMHI_FS_STATUS;
    mmhiData.statusMessage.status = 0;
    mmhiData.statusMessage.mibStatusMask = 0;
    mmhiData.retryCmd = false;
    mmhiData.nackCounter = 0;
    mmhiData.status = MMHI_STATUS_INITIALIZED;
    mmhiData.state = MMHI_STATE_ERROR;
    mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
    mmhiData.tackTimer = SYS_TIME_HANDLE_INVALID;
    mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
    mmhiData.txDelayTimer = SYS_TIME_HANDLE_INVALID;
    mmhiData.rstConfirmSent = false;
    mmhiData.swReset = false;

    /* Init custom commands entries */
    for (cmdTableindex = 0; cmdTableindex < MMHI_MAX_CUSTOM_COMMANDS; cmdTableindex++)
    {
        pCmdTable->commandCode = (uint8_t)MMHI_CMD_INVALID;
        pCmdTable->callback = NULL;
        pCmdTable++;
    }

    return (SYS_MODULE_OBJ) index;
}

/* MISRAC 2012 deviation block end */

MMHI_STATUS MMHI_GetStatus ( SYS_MODULE_OBJ object )
{
    SYS_MODULE_INDEX index = (SYS_MODULE_INDEX)object;

    /* Check Single instance */
    if (index != MMHI_INDEX)
    {
        return MMHI_STATUS_ERROR;
    }

    return mmhiData.status;
}

MMHI_HANDLE MMHI_Open( SYS_MODULE_OBJ object )
{
    SYS_MODULE_INDEX index = (SYS_MODULE_INDEX)object;
    uintptr_t ctx = 0;

    /* Check instance */
    if (index != MMHI_INDEX)
    {
        return MMHI_HANDLE_INVALID;
    }

    /* Check previously initialized */
    if (mmhiData.status != MMHI_STATUS_INITIALIZED)
    {
        return MMHI_HANDLE_INVALID;
    }

    /* Register USART Callbacks */
    mmhiData.uartPLIB->readCallbackRegister(lMMHI_USART_ReadCallback, ctx);

    /* Set EUT as configured and running properly */
    mmhiData.statusMessage.status |= MMHI_STATUS_SET_Msk;

    /* Set MIB objects at default value */
    MMHI_MIB_Initialize();
    mmhiData.statusMessage.mibStatusMask = MMHI_MIB_GetStatus();
    MMHI_MIB_WriteIndCallbackRegister(lMMHI_WriteIndicationCallback);

    /* Clear reception state */
    lMMHI_ClearReceptionState();

    mmhiData.status = MMHI_STATUS_READY;
    mmhiData.startFrameTxMask = 0;

    /* Enable External RTS pin interrupt and register callback */
    SYS_INT_SourceStatusClear(MMHI_EXT_INT_RTS_SRC);
    (void) PIO_PinInterruptCallbackRegister((PIO_PIN)MMHI_EXT_INT_RTS_PIN,
            lMMHI_ExternalInterruptRTSHandler, ctx);
    PIO_PinInterruptEnable((PIO_PIN)MMHI_EXT_INT_RTS_PIN);

    /* EUT shall issue a reset command each time it exits from reset */
    lMMHI_sendResetIndication();

    return (MMHI_HANDLE) index;
}

void MMHI_Tasks ( SYS_MODULE_OBJ object )
{
    SYS_MODULE_INDEX index = (SYS_MODULE_INDEX)object;

    if (index != MMHI_INDEX)
    {
        return;
    }

    MMHI_MIB_Tasks();

    /* Check received frame */
    if (mmhiData.rcvFrameLength > 0U)
    {
        MMHI_RESULT result;

        result = lMMHI_CheckRcvFrame();
        if (result == MMHI_SUCCESS)
        {
            // Process Received Frame
            if (lMMHI_ProcessRcvFrame() == false)
            {
                if (lMMHI_ProcessCustomFrame() == true)
                {
                    /* Send ACK */
                    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_ACK_Msk;
                }
                else
                {
                    /* Send NACK */
                    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_NACK_Msk;
                }
            }
        }
        else
        {
            if (result == MMHI_ERROR_COMMAND)
            {
                uint8_t *pData = mmhiTxBuffer;

                // Send HI Error Command
                *pData++ = (uint8_t)MMHI_FS_COMMAND;
                *pData++ = 0U;
                *pData++ = (uint8_t)MMHI_CMD_HI_ERROR_IND;
                *pData++ = (uint8_t)mmhiData.rcvFrameData.commandCode;

                mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
            }
            else
            {
                // Send NACK
                mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_NACK_Msk;
            }
        }

        mmhiData.rcvFrameLength = 0;
    }

    /* Check pending transmission */
    if (((mmhiData.statusMessage.status & MMHI_STATUS_TX_Msk) == 0U) &&
         (mmhiData.txDelayTimer == SYS_TIME_HANDLE_INVALID))
    {
        if (mmhiData.startFrameTxMask != 0U)
        {
            uint8_t *pData;
            uintptr_t context;
            uint16_t frameLen;
            uint8_t fs;

            if ((mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_STATUS_Msk) != 0U)
            {
                frameLen = (uint16_t)(sizeof(mmhiData.statusMessage));
                context = (uintptr_t) MMHI_FRAME_START_MASK_STATUS_Msk;
                pData = (uint8_t *)&mmhiData.statusMessage;

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> ST\r\n");
            }
            else if ((mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_ACK_Msk) != 0U)
            {
                frameLen = 1;
                context = (uintptr_t) MMHI_FRAME_START_MASK_ACK_Msk;
                fs = (uint8_t)MMHI_FS_ACK;
                pData = (uint8_t *)&fs;

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> ACK\r\n");
            }
            else if ((mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_NACK_Msk) != 0U)
            {
                frameLen = 1;
                context = (uintptr_t) MMHI_FRAME_START_MASK_NACK_Msk;
                fs = (uint8_t)MMHI_FS_NACK;
                pData = (uint8_t *)&fs;
                mmhiData.nackCounter++;
                if (mmhiData.nackCounter >= 2U)
                {
                    lMMHI_ClearReceptionState();
                }

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> NACK\r\n");
            }
            else if ((mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_CMD_Msk) != 0U)
            {
                uint16_t crcCalc;
                uint8_t crcDataLen;
                MMHI_COMMAND command;

                mmhiData.retryCmd = false;

                /* Get command */
                command = (MMHI_COMMAND)mmhiTxBuffer[MMHI_CMD_FRAME_CMD_Pos];

                /* Get payload length */
                frameLen = (uint16_t)mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos] + (uint16_t)1U;

                /* Calculate checksum */
                crcDataLen = (uint8_t)frameLen + 2U;
                crcCalc = (uint16_t)SRV_PCRC_GetValue(&mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos],
                            (size_t)crcDataLen, PCRC_HT_GENERIC, PCRC_CRC16, 0);
                pData = &mmhiTxBuffer[MMHI_CMD_FRAME_PAYLOAD_Pos] + frameLen;
                *pData++ = (uint8_t)crcCalc;
                *pData = (uint8_t)(crcCalc >> 8U);

                /* Get total frame length */
                frameLen += 5U;
                context = (uintptr_t) MMHI_FRAME_START_MASK_CMD_Msk;
                pData = mmhiTxBuffer;

                if (command != MMHI_CMD_BIO_RESET_IND)
                {
                    /* Launch TACK timer */
                    mmhiData.tackTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TackTimerCallback, context,
                            MMHI_PROTOCOL_TIMEOUT_TACK_MS, SYS_TIME_SINGLE);

                }

                /* Check whether reset is pending and confirm is to be sent */
                if ((mmhiData.swReset == true) && (mmhiData.rstConfirmSent == false))
                {
                    /* Set flag */
                    mmhiData.rstConfirmSent = true;
                }

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> CMD\r\n");
            }
            else if ((mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_CMDRET_Msk) != 0U)
            {
                mmhiData.retryCmd = true;

                frameLen = (uint16_t)mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos] + 6U;
                context = (uintptr_t) MMHI_FRAME_START_MASK_CMDRET_Msk;
                pData = mmhiTxBuffer;
                /* Adjust Frame Start */
                *pData = (uint8_t)MMHI_FS_COMMAND_RETRY;

                /* Finish reception state */
                lMMHI_ClearReceptionState();

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> CMDR\r\n");
            }
            else
            {
                /* Not valid frame start type */
                frameLen = 0;
            }

            if (frameLen > 0U)
            {
                mmhiData.statusMessage.status |= MMHI_STATUS_TX_Msk;
                mmhiData.uartPLIB->writeCallbackRegister(lMMHI_USART_WriteCallback, context);
                (void) mmhiData.uartPLIB->writeFn(pData, frameLen);

                /* Launch timer to drive delay between 2 transmissions */
                mmhiData.txDelayTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TxDelayTimerCallback,
                    context, MMHI_PROTOCOL_TIMEOUT_TXD_MS, SYS_TIME_SINGLE);

            }
        }
    }
}

void MMHI_MacDataCallbackRegister(MMHI_MAC_DATA_IND_CALLBACK callback)
{
    mmhiData.macDataCallback = callback;
}

MMHI_RESULT MMHI_CommandCallbackRegister(uint8_t cmdCode, MMHI_CMD_FRAME_IND_CALLBACK callback)
{
    MMHI_CUSTOM_CMD_DATA* pCmdTable = sMmhiCustomCmdData;
    uint8_t index;

    if (callback == NULL)
    {
        /* Free command entry */
        for (index = 0; index < MMHI_MAX_CUSTOM_COMMANDS; index++)
        {
            if (pCmdTable->commandCode == cmdCode)
            {
                pCmdTable->commandCode = (uint8_t)MMHI_CMD_INVALID;
                return MMHI_SUCCESS;
            }

            pCmdTable++;
        }

        return MMHI_ERROR;
    }
    else
    {
        /* Find free command entry and fill */
        for (index = 0; index < MMHI_MAX_CUSTOM_COMMANDS; index++)
        {
            if (pCmdTable->commandCode == (uint8_t)MMHI_CMD_INVALID)
            {
                pCmdTable->commandCode = cmdCode;
                pCmdTable->callback = callback;
                return MMHI_SUCCESS;
            }

            pCmdTable++;
        }

        return MMHI_ERROR;
    }
}

MMHI_RESULT MMHI_SendCommandFrame(uint8_t cmdCode, uint8_t* data, uint8_t length)
{
    uint8_t *pData = mmhiTxBuffer;

    if (mmhiData.startFrameTxMask != 0U)
    {
        return MMHI_ERROR_BUSY;
    }

    *pData++ = (uint8_t)MMHI_FS_COMMAND;
    *pData++ = length - 1U;
    *pData++ = cmdCode;
    (void) memcpy(pData, data, length);

    /* Send Command message */
    mmhiData.startFrameTxMask = MMHI_FRAME_START_MASK_CMD_Msk;

    return MMHI_SUCCESS;
}

/*******************************************************************************
 End of File
*/