/*******************************************************************************
  Meters&More AL Serialization Source File

  Company:
    Microchip Technology Inc.

  File Name:
    al_serial.c

  Summary:
    Meters&More AL Serialization Source File.

  Description:
    The Meters&More AL Serialization allows to serialize the AL API through
    USI interface in order to run the application on an external device. This
    file contains the implementation of the Meters&More AL Serialization.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2025, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
#include <string.h>
#include "al_serial.h"
#include "configuration.h"
#include "service/usi/srv_usi.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    /* Generic messages */
    AL_SERIAL_MSG_STATUS = 0,

    /* AL access request messages */
    AL_SERIAL_MSG_AL_DATA_REQUEST = 10,
    AL_SERIAL_MSG_AL_SET_REQUEST,
    AL_SERIAL_MSG_AL_GET_REQUEST,
    AL_SERIAL_MSG_AL_PERFORM_ECB_REQUEST,

    /* AL response/indication messages */
    AL_SERIAL_MSG_AL_DATA_CONFIRM = 20,
    AL_SERIAL_MSG_AL_DATA_INDICATION,
    AL_SERIAL_MSG_AL_EVENT_INDICATION,
    AL_SERIAL_MSG_AL_SET_CONFIRM,
    AL_SERIAL_MSG_AL_GET_CONFIRM,
    AL_SERIAL_MSG_AL_PERFORM_ECB_CONFIRM,

} AL_SERIAL_MSG_ID;

typedef enum
{
    AL_SERIAL_STATUS_SUCCESS = 0,
    AL_SERIAL_STATUS_NOT_ALLOWED,
    AL_SERIAL_STATUS_UNKNOWN_COMMAND,
    AL_SERIAL_STATUS_INVALID_PARAMETER

} AL_SERIAL_STATUS;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* USI handle for AL Serialization */
static SRV_USI_HANDLE alSeriallUsiHandle;

/* Buffer to send commands through USI */
static uint8_t alSeriallRspBuffer[MAX_LENGTH_432_DATA + 32];

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static void lMemcpyToUsiEndianessUint64(uint8_t* pDst, uint8_t* pSrc)
{
    uint64_t aux;

    (void) memcpy((uint8_t *) &aux, pSrc, 8);

    *pDst++ = (uint8_t) (aux >> 56);
    *pDst++ = (uint8_t) (aux >> 48);
    *pDst++ = (uint8_t) (aux >> 40);
    *pDst++ = (uint8_t) (aux >> 32);
    *pDst++ = (uint8_t) (aux >> 24);
    *pDst++ = (uint8_t) (aux >> 16);
    *pDst++ = (uint8_t) (aux >> 8);
    *pDst = (uint8_t) aux;
}

static void lMemcpyToUsiEndianessUint32(uint8_t* pDst, uint8_t* pSrc)
{
    uint32_t aux;

    (void) memcpy((uint8_t *) &aux, pSrc, 4);

    *pDst++ = (uint8_t) (aux >> 24);
    *pDst++ = (uint8_t) (aux >> 16);
    *pDst++ = (uint8_t) (aux >> 8);
    *pDst = (uint8_t) aux;
}

static void lMemcpyToUsiEndianessUint16(uint8_t* pDst, uint8_t* pSrc)
{
    uint16_t aux;

    (void) memcpy((uint8_t *) &aux, pSrc, 2);

    *pDst++ = (uint8_t) (aux >> 8);
    *pDst = (uint8_t) aux;
}

static void lMemcpyFromUsiEndianessUint64(uint8_t* pDst, uint8_t* pSrc)
{
    uint64_t aux;

    aux = ((uint64_t) *pSrc++) << 56;
    aux = ((uint64_t) *pSrc++) << 48;
    aux = ((uint64_t) *pSrc++) << 40;
    aux = ((uint64_t) *pSrc++) << 32;
    aux = ((uint64_t) *pSrc++) << 24;
    aux += ((uint64_t) *pSrc++) << 16;
    aux += ((uint64_t) *pSrc++) << 8;
    aux += (uint64_t)*pSrc;

    (void) memcpy(pDst, (uint8_t *)&aux, 8U);
}

static void lMemcpyFromUsiEndianessUint32(uint8_t* pDst, uint8_t* pSrc)
{
    uint32_t aux;

    aux = ((uint32_t) *pSrc++) << 24;
    aux += ((uint32_t) *pSrc++) << 16;
    aux += ((uint32_t) *pSrc++) << 8;
    aux += (uint32_t)*pSrc;

    (void) memcpy(pDst, (uint8_t *)&aux, 4U);
}

static void lMemcpyFromUsiEndianessUint16(uint8_t* pDst, uint8_t* pSrc)
{
    uint16_t aux;

    aux = ((uint16_t) *pSrc++) << 8;
    aux += (uint16_t) *pSrc;

    (void) memcpy(pDst, (uint8_t *)&aux, 2U);
}

static void lAL_SER_StringifyMsgStatus(AL_SERIAL_STATUS status, AL_SERIAL_MSG_ID command)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_STATUS;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) status;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) command;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifyDataConfirm(AL_DATA_CONFIRM_PARAMS* cfmParams)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_DATA_CONFIRM;
    (void) memcpy(&alSeriallRspBuffer[serialRspLen], cfmParams->dstAddress.address, MAC_ADDRESS_SIZE);
    serialRspLen += MAC_ADDRESS_SIZE;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) cfmParams->dsap;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) cfmParams->ecc;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) cfmParams->txStatus;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifyDataIndication(AL_DATA_IND_PARAMS *indParams)
{
    uint16_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_DATA_INDICATION;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 56);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 48);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 40);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 32);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 24);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 16);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->datetime >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->datetime;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 56);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 48);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 40);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 32);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 24);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 16);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->lmon >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->lmon;
    (void) memcpy(&alSeriallRspBuffer[serialRspLen], indParams->srcAddress.address, MAC_ADDRESS_SIZE);
    serialRspLen += MAC_ADDRESS_SIZE;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->attr;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->dsap;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->ecc;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->rxStatus;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (indParams->apduLen >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) indParams->apduLen;
    (void) memcpy(&alSeriallRspBuffer[serialRspLen], indParams->apdu, indParams->apduLen);
    serialRspLen += indParams->apduLen;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifyEventIndication(AL_EVENT_IND_PARAMS *eventParams)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_EVENT_INDICATION;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) eventParams->eventId;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) eventParams->eventValue.length;
    (void) memcpy(&alSeriallRspBuffer[serialRspLen], eventParams->eventValue.value, eventParams->eventValue.length);
    serialRspLen += eventParams->eventValue.length;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifySetConfirm(AL_RESULT setResult, uint32_t attributeId, uint16_t attributeIndex)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_SET_CONFIRM;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) setResult;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 24);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 16);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) attributeId;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeIndex >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) attributeIndex;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifyGetConfirm(uint32_t attributeId, uint16_t attributeIndex, AL_IB_VALUE* attributeValue, AL_RESULT result)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_GET_CONFIRM;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) result;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 24);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 16);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeId >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) attributeId;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (attributeIndex >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) attributeIndex;
    alSeriallRspBuffer[serialRspLen++] = attributeValue->length;

    if (result == AL_SUCCESS)
    {
        switch (attributeValue->length)
        {
            /* 8-bit IBs */
            case 1:
                alSeriallRspBuffer[serialRspLen] = attributeValue->value[0];
                break;

            /* 16-bit IBs */
            case 2:
                lMemcpyToUsiEndianessUint16(&alSeriallRspBuffer[serialRspLen], attributeValue->value);
                break;

            /* 32-bit IBs */
            case 4:
                lMemcpyToUsiEndianessUint32(&alSeriallRspBuffer[serialRspLen], attributeValue->value);
                break;

            /* 64-bit IBs */
            case 8:
                lMemcpyToUsiEndianessUint64(&alSeriallRspBuffer[serialRspLen], attributeValue->value);
                break;

            /* Array IBs */
            default:
                (void) memcpy(&alSeriallRspBuffer[serialRspLen], attributeValue->value, attributeValue->length);
                break;
        }
    }

    /* Send through USI */
    serialRspLen += attributeValue->length;
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static void lAL_SER_StringifyPerformECBConfirm(AL_RESULT result, uint8_t *encryptedData, uint16_t dataLen)
{
    uint8_t serialRspLen = 0U;

    /* Fill serial response buffer */
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) AL_SERIAL_MSG_AL_PERFORM_ECB_CONFIRM;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) result;
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) (dataLen >> 8);
    alSeriallRspBuffer[serialRspLen++] = (uint8_t) dataLen;
    (void) memcpy(&alSeriallRspBuffer[serialRspLen], encryptedData, dataLen);
    serialRspLen += dataLen;

    /* Send through USI */
    SRV_USI_Send_Message(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, alSeriallRspBuffer, serialRspLen);
}

static AL_SERIAL_STATUS lAL_SER_ParseDataRequest(uint8_t* pData)
{
    AL_DATA_REQUEST_PARAMS dataReqParams;

    if (AL_GetStatus() < SYS_STATUS_READY)
    {
        /* AL not initialized */
        return AL_SERIAL_STATUS_NOT_ALLOWED;
    }

    /* Parse data request message */
    dataReqParams.datetime = ((uint64_t) *pData++) << 56;
    dataReqParams.datetime += ((uint64_t) *pData++) << 48;
    dataReqParams.datetime += ((uint64_t) *pData++) << 40;
    dataReqParams.datetime += ((uint64_t) *pData++) << 32;
    dataReqParams.datetime += ((uint64_t) *pData++) << 24;
    dataReqParams.datetime += ((uint64_t) *pData++) << 16;
    dataReqParams.datetime += ((uint64_t) *pData++) << 8;
    dataReqParams.datetime += (uint64_t) *pData++;
    dataReqParams.lmon = ((uint64_t) *pData++) << 56;
    dataReqParams.lmon += ((uint64_t) *pData++) << 48;
    dataReqParams.lmon += ((uint64_t) *pData++) << 40;
    dataReqParams.lmon += ((uint64_t) *pData++) << 32;
    dataReqParams.lmon += ((uint64_t) *pData++) << 24;
    dataReqParams.lmon += ((uint64_t) *pData++) << 16;
    dataReqParams.lmon += ((uint64_t) *pData++) << 8;
    dataReqParams.lmon += (uint64_t) *pData++;
    dataReqParams.maxResponseLen = ((uint16_t) *pData++) << 8;
    dataReqParams.maxResponseLen += (uint16_t) *pData++;
    dataReqParams.timeSlotNum = ((uint16_t) *pData++) << 8;
    dataReqParams.timeSlotNum += (uint16_t) *pData++;
    dataReqParams.dstAddress.routeSize = *pData++;
    for (uint8_t i = 0; i < dataReqParams.dstAddress.routeSize; i++)
    {
        (void) memcpy(dataReqParams.dstAddress.macAddress[i].address, pData, MAC_ADDRESS_SIZE);
        pData += MAC_ADDRESS_SIZE;
    }
    dataReqParams.serviceClass = (SERVICE_CLASS) *pData++;
    dataReqParams.attr = (AL_MSG_ATTR) *pData++;
    dataReqParams.dsap = (DLL_DSAP) *pData++;
    dataReqParams.ecc = (DLL_ECC) *pData++;
    dataReqParams.apduLen = ((uint16_t) *pData++) << 8;
    dataReqParams.apduLen += (uint16_t) *pData++;
    dataReqParams.apdu = pData;

    /* Send data request to AL */
    AL_DataRequest(&dataReqParams);

    return AL_SERIAL_STATUS_SUCCESS;
}

static AL_SERIAL_STATUS lAL_SER_ParseSetRequest(uint8_t* pData)
{
    uint32_t attributeIdAux;
    uint16_t attributeIndex;
    AL_IB_ATTRIBUTE attributeId;
    AL_IB_VALUE attributeValue;
    AL_RESULT result;

    if (AL_GetStatus() < SYS_STATUS_READY)
    {
        /* AL not initialized */
        return AL_SERIAL_STATUS_NOT_ALLOWED;
    }

    /* Parse AL set request message */
    attributeIdAux = ((uint32_t) *pData++) << 24;
    attributeIdAux += ((uint32_t) *pData++) << 16;
    attributeIdAux += ((uint32_t) *pData++) << 8;
    attributeIdAux += (uint32_t) *pData++;
    attributeId = (AL_IB_ATTRIBUTE) attributeIdAux;
    attributeIndex = ((uint16_t) *pData++) << 8;
    attributeIndex += (uint16_t) *pData++;
    attributeValue.length = *pData++;

    switch (attributeValue.length)
    {
        /* 8-bit IBs */
        case 1:
            attributeValue.value[0] = *pData;
            break;

        /* 16-bit IBs */
        case 2:
            lMemcpyFromUsiEndianessUint16(attributeValue.value, pData);
            break;

        /* 32-bit IBs */
        case 4:
            lMemcpyFromUsiEndianessUint32(attributeValue.value, pData);
            break;

        /* 64-bit IBs */
        case 8:
            lMemcpyFromUsiEndianessUint64(attributeValue.value, pData);
            break;

        /* Array IBs */
        default:
            (void) memcpy(attributeValue.value, pData, attributeValue.length);
            break;
    }

    /* Send set request to AL */
    result = AL_SetRequest(attributeId, attributeIndex, &attributeValue);

    /* Serialize Set Confirm */
    lAL_SER_StringifySetConfirm(result, attributeIdAux, attributeIndex);

    return AL_SERIAL_STATUS_SUCCESS;
}

static AL_SERIAL_STATUS lAL_SER_ParseGetRequest(uint8_t* pData)
{
    uint32_t attributeId;
    uint16_t attributeIndex;
    AL_IB_VALUE attributeValue;
    AL_RESULT result;

    if (AL_GetStatus() < SYS_STATUS_READY)
    {
        /* AL not initialized */
        return AL_SERIAL_STATUS_NOT_ALLOWED;
    }

    /* Parse AL get request */
    attributeId = ((uint32_t) *pData++) << 24;
    attributeId += ((uint32_t) *pData++) << 16;
    attributeId += ((uint32_t) *pData++) << 8;
    attributeId += (uint32_t) *pData++;

    attributeIndex = ((uint16_t) *pData++) << 8;
    attributeIndex += (uint16_t) *pData;

    /* Send get request to AL */
    result = AL_GetRequest((AL_IB_ATTRIBUTE) attributeId, attributeIndex, &attributeValue);

    /* Serialize Get Confirm */
    lAL_SER_StringifyGetConfirm(attributeId, attributeIndex, &attributeValue, result);

    return AL_SERIAL_STATUS_SUCCESS;
}

static AL_SERIAL_STATUS lAL_SER_ParsePerformEcbRequest(uint8_t* pData)
{
    uint8_t rawData[64];
    uint8_t encryptedData[64];
    uint8_t aesKey[16];
    uint8_t dataLen, keyLen;
    AL_RESULT result;

    if (AL_GetStatus() < SYS_STATUS_READY)
    {
        /* AL not initialized */
        return AL_SERIAL_STATUS_NOT_ALLOWED;
    }

    /* Parse AL Perform ECB request */
    dataLen = *pData++;
    if (dataLen > 64)
    {
        return AL_SERIAL_STATUS_INVALID_PARAMETER;
    }
    (void) memcpy(rawData, pData, dataLen);
    pData += dataLen;
    keyLen = *pData++;
    if (keyLen > 16)
    {
        return AL_SERIAL_STATUS_INVALID_PARAMETER;
    }
    (void) memcpy(aesKey, pData, keyLen);

    /* Send Perform ECB Request to AL */
    result = AL_PerformECB(rawData, dataLen, encryptedData, aesKey, keyLen);
    
    /* Serialize Perform ECB Confirm */
    lAL_SER_StringifyPerformECBConfirm(result, encryptedData, dataLen);

    return AL_SERIAL_STATUS_SUCCESS;
}

static void lAL_SER_CallbackUsiAlProtocol(uint8_t* pData, size_t length)
{
    uint8_t commandAux;
    AL_SERIAL_MSG_ID command;
    AL_SERIAL_STATUS status = AL_SERIAL_STATUS_UNKNOWN_COMMAND;

    /* Protection for invalid length */
    if (length == 0U)
    {
        return;
    }

    /* Process received message */
    commandAux = (*pData++) & 0x7FU;
    command = (AL_SERIAL_MSG_ID) commandAux;

    switch (command)
    {
        case AL_SERIAL_MSG_AL_DATA_REQUEST:
            status = lAL_SER_ParseDataRequest(pData);
            break;

        case AL_SERIAL_MSG_AL_SET_REQUEST:
            status = lAL_SER_ParseSetRequest(pData);
            break;

        case AL_SERIAL_MSG_AL_GET_REQUEST:
            status = lAL_SER_ParseGetRequest(pData);
            break;

        case AL_SERIAL_MSG_AL_PERFORM_ECB_REQUEST:
            status = lAL_SER_ParsePerformEcbRequest(pData);
            break;

        default:
            status = AL_SERIAL_STATUS_UNKNOWN_COMMAND;
            break;
    }

    /* Send status only if there is a processing error */
    if (status != AL_SERIAL_STATUS_SUCCESS)
    {
        lAL_SER_StringifyMsgStatus(status, command);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Interface Function Definitions
// *****************************************************************************
// *****************************************************************************

SYS_MODULE_OBJ AL_SERIAL_Initialize(const SYS_MODULE_INDEX index)
{
    /* Validate the request */
    if (index >= MM_AL_SERIAL_INSTANCES_NUMBER)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Initialize variables */
    alSeriallUsiHandle = SRV_USI_HANDLE_INVALID;

    /* Set AL callbacks */
    AL_DataIndicationCallbackRegister(lAL_SER_StringifyDataIndication);
    AL_DataConfirmCallbackRegister(lAL_SER_StringifyDataConfirm);
    AL_EventIndicationCallbackRegister(lAL_SER_StringifyEventIndication);

    return (SYS_MODULE_OBJ) MM_AL_SERIAL_INDEX_0;
}

void AL_SERIAL_Tasks(SYS_MODULE_OBJ object)
{
    if (object != (SYS_MODULE_OBJ) MM_AL_SERIAL_INDEX_0)
    {
        /* Invalid object */
        return;
    }

    if (alSeriallUsiHandle == SRV_USI_HANDLE_INVALID)
    {
        /* Open USI instance for MAC serialization and register callback */
        alSeriallUsiHandle = SRV_USI_Open(MM_AL_SERIAL_USI_INDEX);
        SRV_USI_CallbackRegister(alSeriallUsiHandle, SRV_USI_PROT_ID_MM_AL_API, lAL_SER_CallbackUsiAlProtocol);
    }
}
