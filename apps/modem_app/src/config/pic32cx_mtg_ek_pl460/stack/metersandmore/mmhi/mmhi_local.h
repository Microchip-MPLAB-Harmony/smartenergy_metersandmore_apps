/*******************************************************************************
  Meters and More Host Interface Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    MMHI_definitions.h

  Summary:
    Meters and More Host Interface local implementation.

  Description:
    This file contains the definitions required by the MMHI interface.
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

#ifndef MMHI_LOCAL_H    /* Guard against multiple inclusion */
#define MMHI_LOCAL_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stack/metersandmore/mmhi/mmhi_mib.h"
#include "system/time/sys_time.h"
#include "peripheral/flexcom/usart/plib_flexcom7_usart.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

#define MMHI_USART_ERROR_NONE      FLEXCOM_USART_ERROR_NONE
#define MMHI_USART_ERROR_OVERRUN   FLEXCOM_USART_ERROR_OVERRUN
#define MMHI_USART_ERROR_PARITY    FLEXCOM_USART_ERROR_PARITY
#define MMHI_USART_ERROR_FRAMING   FLEXCOM_USART_ERROR_FRAMING

/* MM Host Interface Protocol deifinitions */
/* Max length of the frame */
#define MMHI_FRAME_MAX_LENGTH                    (261U)

/* Max length of the frame */
#define MMHI_FRAME_MAX_PAYLOAD_LENGTH            (256U)

/* Command Frame Description */
#define MMHI_CMD_FRAME_FS_Pos                    _UINT8_(0x0U)
#define MMHI_CMD_FRAME_LEN_Pos                   _UINT8_(0x1U)
#define MMHI_CMD_FRAME_CMD_Pos                   _UINT8_(0x2U)
#define MMHI_CMD_FRAME_PAYLOAD_Pos               _UINT8_(0x3U)

/* Status Message: Status field description */
#define MMHI_STATUS_SET_Pos                      _UINT8_(0x0U)
#define MMHI_STATUS_SET_Msk                      (_UINT8_(0x1U) << MMHI_STATUS_SET_Pos)
#define MMHI_STATUS_SET(value)                   (MMHI_STATUS_SET_Msk & (_UINT8_(value) << MMHI_STATUS_SET_Pos))
#define   MMHI_STATUS_SET_CONFIG_ISSUE_Val        _UINT8_(0x0U)
#define   MMHI_STATUS_SET_CONFIG_OK_Val           _UINT8_(0x1U)
#define MMHI_STATUS_TX_Pos                       _UINT8_(0x1U)
#define MMHI_STATUS_TX_Msk                       (_UINT8_(0x1U) << MMHI_STATUS_TX_Pos)
#define MMHI_STATUS_TX(value)                    (MMHI_STATUS_TX_Msk & (_UINT8_(value) << MMHI_STATUS_TX_Pos))
#define   MMHI_STATUS_TX_NOTX_Val                 _UINT8_(0x0U)
#define   MMHI_STATUS_TX_INTX_Val                 _UINT8_(0x1U)
#define MMHI_STATUS_RX_Pos                       _UINT8_(0x2U)
#define MMHI_STATUS_RX_Msk                       (_UINT8_(0x1U) << MMHI_STATUS_RX_Pos)
#define MMHI_STATUS_RX(value)                    (MMHI_STATUS_RX_Msk & (_UINT8_(value) << MMHI_STATUS_RX_Pos))
#define   MMHI_STATUS_TX_NORX_Val                 _UINT8_(0x0U)
#define   MMHI_STATUS_TX_INRX_Val                 _UINT8_(0x1U)
#define MMHI_STATUS_PLC_BUSY_Pos                  _UINT8_(0x3U)
#define MMHI_STATUS_PLC_BUSY_Msk                  (_UINT8_(0x1U) << MMHI_STATUS_PLC_BUSY_Pos)
#define MMHI_STATUS_PLC_BUSY(value)               (MMHI_STATUS_PLC_BUSY_Msk & (_UINT8_(value) << MMHI_STATUS_PLC_BUSY_Pos))
#define   MMHI_STATUS_PLC_IDLE_Val                _UINT8_(0x0U)
#define   MMHI_STATUS_PLC_BUSY_Val                _UINT8_(0x1U)

/* MMHI Protocol Timeouts in Milliseconds */
/* Inter Character timeout */
#define MMHI_PROTOCOL_TIMEOUT_TIC_MS              (10U)
/* Acknowledge timeout */
#define MMHI_PROTOCOL_TIMEOUT_TACK_MS             (1000U)   // 40 !!!!!!!
/* Command timeout after Modem Status */
#define MMHI_PROTOCOL_TIMEOUT_TSR_MS              (1250U)  // 125 !!!!!!!
/* Delay between 2 consecutive transmissions */
#define MMHI_PROTOCOL_TIMEOUT_TXD_MS              (MMHI_PROTOCOL_TIMEOUT_TIC_MS + 2U)

/* Frame Start Command Mask description */
#define MMHI_FRAME_START_MASK_CMD_Pos             _UINT8_(0U)
#define MMHI_FRAME_START_MASK_CMD_Msk             (_UINT8_(0x1U) << MMHI_FRAME_START_MASK_CMD_Pos)
#define MMHI_FRAME_START_MASK_CMDRET_Pos          _UINT8_(1U)
#define MMHI_FRAME_START_MASK_CMDRET_Msk          (_UINT8_(0x1U) << MMHI_FRAME_START_MASK_CMDRET_Pos)
#define MMHI_FRAME_START_MASK_STATUS_Pos          _UINT8_(2U)
#define MMHI_FRAME_START_MASK_STATUS_Msk          (_UINT8_(0x1U) << MMHI_FRAME_START_MASK_STATUS_Pos)
#define MMHI_FRAME_START_MASK_ACK_Pos             _UINT8_(3U)
#define MMHI_FRAME_START_MASK_ACK_Msk             (_UINT8_(0x1U) << MMHI_FRAME_START_MASK_ACK_Pos)
#define MMHI_FRAME_START_MASK_NACK_Pos            _UINT8_(4U)
#define MMHI_FRAME_START_MASK_NACK_Msk            (_UINT8_(0x1U) << MMHI_FRAME_START_MASK_NACK_Pos)

// *****************************************************************************
/* Meters And More HI Command Frame Start Bytes

  Summary:
    Command Frame Start / Restart

  Description:
    Used to mark the start of frame: 0x02 is used for first packet transmission, 
    0x03 is used for retransmission (if receiving NAK or not receiving ACK).
*/
typedef enum
{
    MMHI_FS_COMMAND = 0x02U,
    MMHI_FS_COMMAND_RETRY = 0x03U,
    MMHI_FS_STATUS = 0x3FU,
    MMHI_FS_ACK = 0x06U,
    MMHI_FS_NACK = 0x15U,
 
} MMHI_CMD_FRAME_START;

// *****************************************************************************
/* Meters And More HI Command Frame

  Summary:
    Lower Layer Basic Component Status Message

  Description:
    The status message is a frame sent by the EUT to notify the host controller 
    its availability to receive and process a frame
*/
typedef struct
{
    /* Frame Start / Restart */
    MMHI_CMD_FRAME_START frameStart;
    /* Number of bytes of the Payload field - 1. (0...255) */
    uint8_t length;
    /* Identifies a specific command */
    MMHI_COMMAND commandCode;
    /* Pointer to buffer where store the payload field format depending on the type of command */
    uint8_t* pPayload;
    /* Checksum (BYTE checksum) computed on Length, Command code and Payload field */
    uint16_t checksum;

} MMHI_CMD_FRAME;

// *****************************************************************************
/* Meters And More HI Status Message

  Summary:
    Lower Layer Basic Component Status Message

  Description:
    The status message is a frame sent by the EUT to notify the host controller 
    its availability to receive and process a frame
*/
typedef struct
{
    /* Status Message Start */
    MMHI_CMD_FRAME_START frameStart;
    /* Status description */
    uint8_t status;
    /* MIB objects status mask. */
    uint16_t mibStatusMask;

} MMHI_STATUS_MESSAGE;

// *****************************************************************************
/* MMHI module internal State

  Summary:
    Defines the internal state of the MMHI module.

  Description:
    This enumeration defines the status of the MMHI module:
        - MMHI_STATE_IDLE: MMHI module has not been initialized.
        - MMHI_STATE_TRANSMITTING: MMHI module has initialized but not opened
        - MMHI_STATE_RECEIVING: MMHI module is ready to be used.
        - MMHI_STATE_ERROR: An unspecified error has occurred
  Remarks:
    None.
*/
typedef enum
{
    MMHI_STATE_IDLE = 0U,
    MMHI_STATE_TRANSMITTING,
    MMHI_STATE_RECEIVING,
    MMHI_STATE_ERROR,
} MMHI_STATE;

typedef struct
{
    /* Initialization data for the UART device */
    MMHI_PLIB_INTERFACE* uartPLIB;

    /* Pointer to handle reception */
    uint8_t* pReceiveData;

    /* Serial received byte */
    uint8_t rcvByte;

    /* Serial received byte */
    uint8_t rcvFrameLength;

    /* MMHI status */
    MMHI_STATUS status;

    /* MMHI internal state */
    MMHI_STATE state;

    /* Mask interface to start new frame transmission */
    uint8_t startFrameTxMask;

    /* Status message */
    MMHI_STATUS_MESSAGE statusMessage;
    
    /* Received frame data */
    MMHI_CMD_FRAME rcvFrameData;

    /* Retry flag */
    bool retryCmd;
    
    /* Inter Character Timer handler */
    SYS_TIME_HANDLE ticTimer;
    
    /* Acknowledge Timer handler */
    SYS_TIME_HANDLE tackTimer;
    
    /* Command timeout after Modem Status Timer handler */
    SYS_TIME_HANDLE tsrTimer;
    
    /* Delay Time between 2 consecutive transmissions */
    SYS_TIME_HANDLE txDelayTimer;
    
    /* NACK counter */
    uint8_t nackCounter;

    /* MIB Response Data */
    MMHI_MIB_DATA mibData;
    
    /* Flag to indicate that SW reset must be performed */
    bool swReset;

    /* Callback function to handle MAC DATA commands */
    MMHI_MAC_DATA_IND_CALLBACK macDataCallback;
    
} MMHI_DATA;

// *****************************************************************************
/* Meters And More HI Custom Command Data

  Summary:
    Data to store relation between custom commands and callbacks routine to be raised.

  Description:
    The status message is a frame sent by the EUT to notify the host controller 
    its availability to receive and process a frame
*/
typedef struct
{
    /* Identifies a specific command */
    uint8_t commandCode;
    /* Pointer to callback function */
    MMHI_CMD_FRAME_IND_CALLBACK callback;

} MMHI_CUSTOM_CMD_DATA;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus

    }

#endif
// DOM-IGNORE-END

#endif /* MMHI_LOCAL_H */