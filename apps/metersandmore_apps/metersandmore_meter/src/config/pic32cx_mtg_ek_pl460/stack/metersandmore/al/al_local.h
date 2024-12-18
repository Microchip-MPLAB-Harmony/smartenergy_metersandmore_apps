/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    al_local.h

  Summary:
    Meters And More AL (Application Layer) Local header file.

  Description:
    Meters And More AL (Application Layer) Local header file. This file
    provides definitions and types internally used by AL.
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

#ifndef AL_LOCAL_H
#define AL_LOCAL_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "al.h"
#include "system/time/sys_time.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

#define AL_INSTANCES_NUMBER     1U

/* Macros for AL_DataRequestHI function */
#define AL_HI_NOR_MASK               0x01U
#define AL_HI_RP_MASK                0x0EU
#define AL_HI_SCA_SIZE               2U
#define AL_HI_CSL_INDEX              5U
#define AL_HI_CSL_VALUE              0x03U

/* AES-CMAC lengths */
#define AL_CMAC_INPUT_LEN            (AL_LMON_LENGTH + 8U) /* 16 bytes */
#define AL_CMAC_OUTPUT_LEN           16U

/* AES-CTR lengths */
#define AL_AES_CTR_IV_LEN            16U

/* Lengths of Network Management messages */
#define AL_NM_ADDRESS_REQ_LEN        4U
#define AL_NM_ADDRESS_RESP_LEN       12U
#define AL_NM_TCT_SET_REQ_LEN        1U
#define AL_NM_REQADDR_RESP_MAX_NODES 4U
#define AL_NM_REQADDR_RESP_MAX_LEN   (1U + (AL_NM_REQADDR_RESP_MAX_NODES * AL_NM_ADDRESS_RESP_LEN))
#define AL_NM_NACK_REQ_LEN           4U

/* Lengths of LMON synchronization messages */
#define AL_CHL_REQ_RANDOM_N_LEN      16U
#define AL_CHL_REQ_LEN               (2U + AL_CHL_REQ_RANDOM_N_LEN)

/* Lengths of Authenticated NACK message */
#define AL_NACK_AUTH_ETM_LEN         8U

/* Default parameters to relay REQADDRESS.req frames */
#define AL_REQ_ADDR_MAX_RESPONSE_LEN 90U
#define AL_REQ_ADDR_TIME_SLOT_NUM    64U

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    AL_STATE_WAIT_DLL_READY,
    AL_STATE_IDLE,
    AL_STATE_WAITING_TX_CFM,
    AL_STATE_WAITING_RX,
    AL_STATE_WAITING_TIMEOUT,
} AL_STATE;

typedef struct
{
    /* LMON counter */
    uint64_t lmon;
    /* Next task time in ticks */
    uint64_t nextTaskTimeCount;
    /* Task rate in SYS_TIME counter ticks */
    uint32_t taskRateCount;
    /* AL Data Indication Callback */
    AL_DATA_IND_CALLBACK dataIndCallback;
    /* AL Data Confirm Callback */
    AL_DATA_CONFIRM_CALLBACK dataCfmCallback;
    /* AL Event Indication Callback */
    AL_EVENT_IND_CALLBACK eventIndCallback;
    /* Handle for Tx Timeout */
    SYS_TIME_HANDLE txTimeoutHandle;
    /* DLL Data Request parameters */
    DLL_DATA_REQUEST_PARAMS dllDataReqParams;
    /* AL Data Confirm (from Tasks) */
    AL_DATA_CONFIRM_PARAMS dataCfmParamsTasks;
    /* Write Key (K1) */
    uint8_t keyWrite[AL_KEY_LENGTH];
    /* Read Key (K2) */
    uint8_t keyRead[AL_KEY_LENGTH];
    /* 128-bit random number N included in CHL.REQ message */
    uint8_t chlReqRandomN[AL_CHL_REQ_RANDOM_N_LEN];
    /* Etm* field for CRC calculation (part of AES-CMAC input) in Authenticated NACK message */
    uint8_t nackCrcEtmField[AL_NACK_AUTH_ETM_LEN];
    /* ACA of slave destination node, used by DCU for Authentication */
    MAC_ADDRESS acaDestination;
    /* Own ACA, used by Meter for Authentication */
    MAC_ADDRESS acaOwn;
    /* Address parameter in last DL_DATA.indication */
    MAC_ADDRESS lastRxAddress;
    /* Status of the AL module */
    SYS_STATUS status;
    /* State of the AL module State Machine */
    AL_STATE state;
    /* ECC (SSAP) field received in last message */
    DLL_ECC eccReceived;
    /* Counter of Tx retries */
    uint16_t txRetryCount;
    /* Limit of Tx retries */
    uint8_t txRetryLimit;
    /* TCT used in Network Management (min 1 to max 255) */
    uint8_t tct;
    /* Length of last received LSDU */
    uint8_t lastlRxLsduLen;
    /* Flag to indicate whether Tx Timeout has expired */
    bool txTimeoutExpired;
    /* Flag to indicate whether response to last protected request is valid. Only for Meter */
    bool responseProtectedValid;
    /* Flag to indicate that a Data Confirm is pending to be notified from Tasks */
    bool dataCfmPending;
    /* Flag to indicate whether Data Confirm needs to be notified to upper layer */
    bool sendDataCfm;
    /* Flag to indicate whether AL_DataRequest is called internally */
    bool alDataReqInternal;
    /* Flag to indicate whether multi-response is expected */
    bool multiresponse;
    /* Flag to indicate whether Read Key is valid */
    bool keyReadValid;
    /* Flag to indicate whether Write Key is valid */
    bool keyWriteValid;
    /* Is master node (false in slave node) */
    bool isMaster;

} AL_DATA;

#ifdef __cplusplus
}
#endif

#endif // #ifndef AL_LOCAL_H

/* *****************************************************************************
 End of File
 */
