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
#include "stack/metersandmore/pal/pal.h"
#include "configuration.h"
#include "osal/osal.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
/* Custom Command Data */
#define MMHI_INVALID_CUSTOM_COMMAND   0xFF 
#define MMHI_MAX_CUSTOM_COMMANDS      5   
static MMHI_CUSTOM_CMD_DATA sMmhiCustomCmdData[MMHI_MAX_CUSTOM_COMMANDS];

/* MM Host Interface Data */
static MMHI_DATA mmhiData = {0};

static uint8_t mmhiTxBuffer[MMHI_FRAME_MAX_LENGTH];
static uint8_t mmhiRxBuffer[MMHI_FRAME_MAX_LENGTH];

static uint8_t mmhiDsapByProtocol[] = {0, 0, 0, 0, 3, 0, 1};

static void lMMHI_ClearReceptionState(void);

// *****************************************************************************
// *****************************************************************************
// Section: Local Callbacks
// *****************************************************************************
// *****************************************************************************
static void lMMHI_TicTimerCallback ( uintptr_t context )
{
    mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
    mmhiData.rcvFrameLength = mmhiData.pReceiveData - mmhiRxBuffer;
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
    BSP_MIKROBUS_1_INT_Off();
    mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
    /* Finish reception state */
    lMMHI_ClearReceptionState();
    
    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Tsr!!!\r\n");
}

static void lMMHI_TxDelayTimerCallback ( uintptr_t context )
{
    mmhiData.txDelayTimer = SYS_TIME_HANDLE_INVALID;
    
    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "TxDelay!!!\r\n");
}

static void lMMHI_USART_ReadCallback( uintptr_t context )
{
    if(MMHI_USART_ERROR_NONE == mmhiData.uartPLIB->errorGet())
    {
        if (mmhiData.state == MMHI_STATE_RECEIVING)
        {
            uint16_t totalRcvLength;

            *mmhiData.pReceiveData++ = mmhiData.rcvByte;
            mmhiData.uartPLIB->read(&mmhiData.rcvByte, 1U);

            /* Cancel TSR timer */
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
                mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
            }

            // Transfer completed successfully
            mmhiData.statusMessage.status |= MMHI_STATUS_RX_Msk;
            totalRcvLength = mmhiData.pReceiveData - mmhiRxBuffer;
            
            if (totalRcvLength == (MMHI_FRAME_MAX_LENGTH - 1U))
            {
                mmhiData.rcvFrameLength = totalRcvLength;
            }
            else
            {
                /* Launch TIC timer */
                if (mmhiData.ticTimer != SYS_TIME_HANDLE_INVALID)
                {
                    SYS_TIME_TimerDestroy(mmhiData.ticTimer);
                    mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
                }

                mmhiData.ticTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TicTimerCallback, (uintptr_t) NULL, 
                            MMHI_PROTOCOL_TIMEOUT_TIC_MS, SYS_TIME_SINGLE);
            }
        }
    }
}

static void lMMHI_USART_WriteCallback( uintptr_t context )
{
    mmhiData.startFrameTxMask &= ~context;
    mmhiData.statusMessage.status &= ~MMHI_STATUS_TX_Msk;
    
    if (mmhiData.swReset == true)
    {
        RSTC_Reset(RSTC_ALL_RESET);
    }

    if (context == MMHI_FRAME_START_MASK_STATUS_Msk)
    {
        /* Start reception and launch TSR timer */
        mmhiData.state = MMHI_STATE_RECEIVING;
        mmhiData.pReceiveData = mmhiRxBuffer;
        mmhiData.uartPLIB->read(&mmhiData.rcvByte, 1U);
        if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
        {
            SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
        }
        mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback, (uintptr_t) NULL, 
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

void lMMHI_WriteIndicationCallback( MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pMibData )
{
    uint8_t *pData = mmhiTxBuffer;

    *pData++ = MMHI_FS_COMMAND;
    *pData++ = pMibData->dataLength;
    *pData++ = MMHI_CMD_MIB_WRITE_IND;
    *pData++ = mibIndex;
    memcpy(pData, pMibData->dataValue, pMibData->dataLength);
    
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
        SYS_TIME_TimerDestroy(mmhiData.ticTimer);
        mmhiData.ticTimer = SYS_TIME_HANDLE_INVALID;
    }
    if (mmhiData.tackTimer != SYS_TIME_HANDLE_INVALID)
    {
        SYS_TIME_TimerDestroy(mmhiData.tackTimer);
        mmhiData.tackTimer = SYS_TIME_HANDLE_INVALID;
    }
    if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
    {
        SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
        mmhiData.tsrTimer = SYS_TIME_HANDLE_INVALID;
    }
}

static uint8_t lMMHI_CheckCustomCommand(MMHI_COMMAND command)
{
    MMHI_CUSTOM_CMD_DATA* pCmdTable = sMmhiCustomCmdData;
    uint8_t index;
    
    for (index = 0; index < MMHI_MAX_CUSTOM_COMMANDS; index++)
    {
        if (command == pCmdTable->commandCode)
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
    bool result = true;

    pRcvFrameData = &mmhiData.rcvFrameData;
    
    if (pRcvFrameData->commandCode == MMHI_CMD_BIO_RESET_REQ)
    {
        uint8_t *pData = mmhiTxBuffer;
        
        *pData++ = MMHI_FS_COMMAND;
        *pData++ = 0;
        *pData++ = MMHI_CMD_BIO_RESET_CFM;
        *pData++ = 0;
        
        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
        
        /* Set reset flag to be performed when TX is completed */
        mmhiData.swReset = true;
        
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
            *pData++ = MMHI_FS_COMMAND;
            *pData++ = 0;
            *pData++ = MMHI_CMD_MIB_WRITE_NCFM;
            *pData++ = MMHI_ERROR_WPL;
        }
        else
        {
            mibIndex = (MMHI_MIB_INDEX)pRcvFrameData->pPayload[0];
            (void) memcpy(pMibData->dataValue, &pRcvFrameData->pPayload[1], mibLength);
            pMibData->dataLength = mibLength;

            result = MMHI_MIB_Set(mibIndex, pMibData, false);

            if (result == MMHI_SUCCESS)
            {
                *pData++ = MMHI_FS_COMMAND;
                *pData++ = 0;
                *pData++ = MMHI_CMD_MIB_WRITE_CFM;
                *pData++ = mibIndex;
                
                mmhiData.statusMessage.mibStatusMask = MMHI_MIB_GetStatus();
            }
            else
            {
                *pData++ = MMHI_FS_COMMAND;
                *pData++ = 0;
                *pData++ = MMHI_CMD_MIB_WRITE_NCFM;
                *pData++ = result;
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

        mibIndex = pRcvFrameData->pPayload[0];

        result = MMHI_MIB_Get(mibIndex, pMibData);

        if (result == MMHI_SUCCESS)
        {
            *pData++ = MMHI_FS_COMMAND;
            *pData++ = pMibData->dataLength - 1;
            *pData++ = MMHI_CMD_MIB_READ_CFM;
            (void) memcpy(pData, pMibData->dataValue, pMibData->dataLength);
        }
        else
        {
            *pData++ = MMHI_FS_COMMAND;
            *pData++ = 0;
            *pData++ = MMHI_CMD_MIB_READ_NCFM;
            *pData++ = result;
        }

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
        
        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- MIB_READ_REQ\r\n");

    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_SLAVE_DATA_REQ)
    {
        uint8_t *pData = pRcvFrameData->pPayload;
        uint8_t protocol;
        uint8_t reqId;

        protocol = *pData++;
        reqId = *pData++;

        if (mmhiData.macDataCallback != NULL)
        {
            mmhiData.macDataCallback(mmhiDsapByProtocol[protocol], reqId, pData, 
                    pRcvFrameData->length - 1);
        }
        
        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- SLAVE_DATA_REQ\r\n");
        
    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_MASTER_DATA_REQ)
    {
        uint8_t *pData = pRcvFrameData->pPayload;
        uint8_t protocol;
        uint8_t reqId;

        protocol = *pData++;
        reqId = *pData++;

        if (mmhiData.macDataCallback != NULL)
        {
            mmhiData.macDataCallback(mmhiDsapByProtocol[protocol], reqId, pData, 
                    pRcvFrameData->length - 1);
        }

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- MASTER_DATA_REQ\r\n");
        
    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_HI_PING_REQ)
    {
        uint8_t *pData = mmhiTxBuffer;

        *pData++ = MMHI_FS_COMMAND;
        *pData++ = pRcvFrameData->length;
        *pData++ = MMHI_CMD_HI_PING_CFM;
        (void) memcpy(pData, pRcvFrameData->pPayload, pRcvFrameData->length + 1);

        mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
        
        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- HI_PING_REQ\r\n");
    }
    else if (pRcvFrameData->commandCode == MMHI_CMD_PLC_PHY_DATA_REQ)
    {
        PAL_MMHI_TxRequest(pRcvFrameData->pPayload, pRcvFrameData->length);

        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- PLC_PHY_DATA_REQ\r\n");
        
    }
    else
    {
        result = false;
    }
    
    return result;
}

static bool lMMHI_ProcessRcvFrame(void)
{
    bool result = true;

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
                SYS_TIME_TimerDestroy(mmhiData.tackTimer);
            }

            /* Start timer to finish reception state */
            BSP_MIKROBUS_1_INT_On();
            
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
            }
            mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback, 
                (uintptr_t) NULL, MMHI_PROTOCOL_TIMEOUT_TSR_MS, SYS_TIME_SINGLE);
            
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
            BSP_MIKROBUS_1_INT_On();
            if (mmhiData.tsrTimer != SYS_TIME_HANDLE_INVALID)
            {
                SYS_TIME_TimerDestroy(mmhiData.tsrTimer);
            }
            mmhiData.tsrTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TsrTimerCallback, 
                (uintptr_t) NULL, MMHI_PROTOCOL_TIMEOUT_TSR_MS, SYS_TIME_SINGLE);
            
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "<- NACK\r\n");
        }
        break;

        default:
            result = false;
    }

    return result;
}

static MMHI_RESULT lMMHI_CheckRcvFrame(void)
{
    MMHI_CMD_FRAME *pRcvFrameData;
    uint8_t *pData;
    uint16_t crcCalc;
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
        crcCalc = (uint16_t)SRV_PCRC_GetValue(&mmhiRxBuffer[MMHI_CMD_FRAME_LEN_Pos], 
                    pRcvFrameData->length + 3U, PCRC_HT_GENERIC, PCRC_CRC16, 0);
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
            if (pRcvFrameData->length != 0)
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
        else if (pRcvFrameData->commandCode == MMHI_CMD_PLC_PHY_DATA_REQ)
        {
            // TBD  
        }
        else
        {
            uint8_t cmdIndex;

            cmdIndex = lMMHI_CheckCustomCommand(mmhiData.rcvFrameData.commandCode);
            if (cmdIndex == MMHI_INVALID_CUSTOM_COMMAND)
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
    uint8_t rstCause; 
    MMHI_RESET_CAUSE mmhcRstCause;
    
    /* Get reset cause */
    rstCause = (uint8_t)(RSTC_ResetCauseGet() >> RSTC_SR_RSTTYP_Pos);
    
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
        
    *pData++ = MMHI_FS_COMMAND;
    *pData++ = 0;
    *pData++ = MMHI_CMD_BIO_RESET_IND;
    /* All reconfigurable MIB objects has been reconfigured */
    /* Reconfiguration status (Bit 7) */
    *pData++ = (1 << 7) | mmhcRstCause;

    mmhiData.startFrameTxMask |= MMHI_FRAME_START_MASK_CMD_Msk;
}

// *****************************************************************************
// *****************************************************************************
// Section: System Interface Functions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 Rule 11.3 deviated:1 Deviation record ID -  H3_MISRAC_2012_R_11_3_DR_1 */

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
    mmhiData.swReset = false;

    /* Init custom commands entries */
    for (cmdTableindex = 0; cmdTableindex < MMHI_MAX_CUSTOM_COMMANDS; cmdTableindex++)
    {
        pCmdTable->commandCode = MMHI_CMD_INVALID;
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
    mmhiData.uartPLIB->readCallbackRegister(lMMHI_USART_ReadCallback, (uintptr_t) NULL);

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
            lMMHI_ExternalInterruptRTSHandler, (uintptr_t) NULL);
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
    if (mmhiData.rcvFrameLength > 0)
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
                *pData++ = MMHI_FS_COMMAND;
                *pData++ = 0U;
                *pData++ = MMHI_CMD_HI_ERROR_IND;
                *pData++ = mmhiData.rcvFrameData.commandCode;

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

            if (mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_STATUS_Msk)
            {
                frameLen = sizeof(mmhiData.statusMessage);
                context = (uintptr_t) MMHI_FRAME_START_MASK_STATUS_Msk;
                pData = (uint8_t *)&mmhiData.statusMessage;
    
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> ST\r\n");
            }
            else if (mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_ACK_Msk)
            {
                frameLen = 1;
                context = (uintptr_t) MMHI_FRAME_START_MASK_ACK_Msk;
                fs = MMHI_FS_ACK;
                pData = (uint8_t *)&fs;
    
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> ACK\r\n");
            }
            else if (mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_NACK_Msk)
            {
                frameLen = 1;
                context = (uintptr_t) MMHI_FRAME_START_MASK_NACK_Msk;
                fs = MMHI_FS_NACK;
                pData = (uint8_t *)&fs;
                mmhiData.nackCounter++;
                if (mmhiData.nackCounter >= 2)
                {
                    lMMHI_ClearReceptionState();
                }
    
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> NACK\r\n");
            }
            else if (mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_CMD_Msk)
            {
                uint16_t crcCalc;
                MMHI_COMMAND command;

                mmhiData.retryCmd = false;
                
                /* Get command */
                command = mmhiTxBuffer[MMHI_CMD_FRAME_CMD_Pos];

                /* Get payload length */
                frameLen = mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos] + 1;

                /* Calculate checksum */
                crcCalc = (uint16_t)SRV_PCRC_GetValue(&mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos], 
                            frameLen + 2U, PCRC_HT_GENERIC, PCRC_CRC16, 0);
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
                    mmhiData.tackTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TackTimerCallback, (uintptr_t) NULL, 
                            MMHI_PROTOCOL_TIMEOUT_TACK_MS, SYS_TIME_SINGLE);
                    
                }

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> CMD\r\n");
            }
            else if (mmhiData.startFrameTxMask & MMHI_FRAME_START_MASK_CMDRET_Msk)
            {
                mmhiData.retryCmd = true;

                frameLen = mmhiTxBuffer[MMHI_CMD_FRAME_LEN_Pos] + 6U;
                context = (uintptr_t) MMHI_FRAME_START_MASK_CMDRET_Msk;
                pData = mmhiTxBuffer;
                /* Adjust Frame Start */
                *pData = MMHI_FS_COMMAND_RETRY;

                /* Finish reception state */
                lMMHI_ClearReceptionState();
    
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "-> CMDR\r\n");
            }
            else
            {
                /* Not valid frame start type */
                frameLen = 0;
            }

            if (frameLen > 0)
            {
                mmhiData.statusMessage.status |= MMHI_STATUS_TX_Msk;
                mmhiData.uartPLIB->writeCallbackRegister(lMMHI_USART_WriteCallback, context);
                (void) mmhiData.uartPLIB->write(pData, frameLen);
                
                /* Launch timer to drive delay between 2 transmissions */
                mmhiData.txDelayTimer = SYS_TIME_CallbackRegisterMS(lMMHI_TxDelayTimerCallback, 
                    (uintptr_t) NULL, MMHI_PROTOCOL_TIMEOUT_TXD_MS, SYS_TIME_SINGLE);
                
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
                pCmdTable->commandCode = MMHI_CMD_INVALID;
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
            if (pCmdTable->commandCode == MMHI_CMD_INVALID)
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
    
    if (length > MMHI_FRAME_MAX_PAYLOAD_LENGTH)
    {
        return MMHI_ERROR;
    }

    if (mmhiData.startFrameTxMask != 0)
    {
        return MMHI_ERROR_BUSY;
    }
    
    *pData++ = MMHI_FS_COMMAND;
    *pData++ = length - 1;
    *pData++ = cmdCode;
    (void) memcpy(pData, data, length);
    
    /* Send Command message */
    mmhiData.startFrameTxMask = MMHI_FRAME_START_MASK_CMD_Msk;
    
    return MMHI_SUCCESS;
}

/*******************************************************************************
 End of File
*/