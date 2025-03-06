/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

#define MAX_TCT_VALUE        0xFF
#define MIN_TCT_VALUE        0x00
#define ADDR_RESP_INFO_LEN   12

APP_DATA appData;

static uint8_t txBuffer[16];

static const MAC_ADDRESS broadcastAddress = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03}};

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void lAPP_BlinkLedTimerCallback (uintptr_t context)
{
    appData.tmrBlinkLedExpired = true;
}

static void lAPP_PlcIndLedTimerCallback (uintptr_t context)
{
    appData.tmrPlcIndLedExpired = true;
}

static void lAPP_StateTimerCallback (uintptr_t context)
{
    appData.tmrStateTimeoutExpired = true;
}

static void lAPP_AL_DataIndicationCallback(AL_DATA_IND_PARAMS *indParams)
{
    ROUTING_ENTRY *rEntry;
    uint8_t idx;
    uint8_t numReportedNodes;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL_DATA_INDICATION: DSAP=0x%02X, ECC=0x%02X, SrcAddr=0x%02X%02X%02X%02X%02X%02X, ATTR=%hhu, APDU=0x",
            indParams->dsap, indParams->ecc,
            indParams->srcAddress.address[0], indParams->srcAddress.address[1], indParams->srcAddress.address[2],
            indParams->srcAddress.address[3], indParams->srcAddress.address[4], indParams->srcAddress.address[5],
            indParams->attr);

    for (uint8_t i = 0; i < indParams->apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", indParams->apdu[i]);
    }

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    if (indParams->dsap == DLL_DSAP_APPLICATION_FRAME)
    {
        if (indParams->rxStatus == AL_RX_STATUS_ERROR_BAD_TMAC)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "TMAC Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_NO_KEY)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Error No Decryption Key\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_DECRYPT)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Decryption Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_CMAC)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Authentication Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_BAD_LEN)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Error Bad Length\r\n");
        }
        else
        {
            if (indParams->attr == AL_MSG_READ_RESP)
            {
                /* Print Info */
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "READ RESPONSE Received from 0x%02X%02X%02X%02X%02X%02X\r\nData=0x",
                        indParams->srcAddress.address[0], indParams->srcAddress.address[1], indParams->srcAddress.address[2],
                        indParams->srcAddress.address[3], indParams->srcAddress.address[4], indParams->srcAddress.address[5]);
                for (uint8_t i = 0; i < indParams->apduLen; i++)
                {
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", indParams->apdu[i]);
                }
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");
            }
            else if (indParams->attr == AL_MSG_READ_RESP_AUTH)
            {
                /* Print Info */
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "READ RESPONSE ENCRYPTED Received from 0x%02X%02X%02X%02X%02X%02X\r\nData=0x",
                        indParams->srcAddress.address[0], indParams->srcAddress.address[1], indParams->srcAddress.address[2],
                        indParams->srcAddress.address[3], indParams->srcAddress.address[4], indParams->srcAddress.address[5]);
                for (uint8_t i = 0; i < indParams->apduLen; i++)
                {
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", indParams->apdu[i]);
                }
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");
            }
            else if (indParams->attr == AL_MSG_NACK_A_NODE_AUTH)
            {
                /* NACK from Node */
                if (indParams->apdu[0] == AL_NACK_AUTH_PAYLOAD)
                {
                    /* LMON mismatch received */
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "LMON mismatch. Use correct LMON received to send frame again\r\n");
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Received LMON 0x%llX\r\n", indParams->lmon);
                    /* Update LMON to received value */
                    appData.lmonTable[appData.numReadEncryptedSent] = indParams->lmon;
                    /* Set flag to send frame again */
                    appData.lmonMismatchReceived = true;
                }
                else
                {
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "NACK_A_NODE_AUTH not recognized\r\n");
                }
            }
            else
            {
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Application Frame command not handled by this APP\r\n");
            }
        }
    }
    else if (indParams->dsap == DLL_DSAP_NETWORK_MANAGEMENT)
    {
        if (indParams->rxStatus == AL_RX_STATUS_ERROR_BAD_TMAC)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "TMAC Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_NO_KEY)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Error No Decryption Key\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_DECRYPT)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Decryption Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_AES_CMAC)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Authentication Error\r\n");
        }
        else if (indParams->rxStatus == AL_RX_STATUS_ERROR_BAD_LEN)
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Error Bad Length\r\n");
        }
        else
        {
            if (indParams->attr == AL_MSG_ADDRESS_RESP)
            {
                /* Get parameters from APDU */
                /* ACA of Discovered Node, Store it in Routing table, but reversed to be used for addressing */
                rEntry = &appData.routingTable[appData.numFoundNodes];
                rEntry->macAddress[0].address[5] = indParams->apdu[0];
                rEntry->macAddress[0].address[4] = indParams->apdu[1];
                rEntry->macAddress[0].address[3] = indParams->apdu[2];
                rEntry->macAddress[0].address[2] = indParams->apdu[3];
                rEntry->macAddress[0].address[1] = indParams->apdu[4];
                rEntry->macAddress[0].address[0] = indParams->apdu[5];
                rEntry->routeSize = 1;
                appData.numFoundNodes++;
                appData.numFoundNodesNew++;
                /* Print Info */
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "ADDRESS RESPONSE Received\r\n");
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "ACA=0x%02X%02X%02X%02X%02X%02X, Av_SIG=%hhu Av_SNR=%hhu, Av_TX=%hhu\r\n",
                        indParams->apdu[0], indParams->apdu[1], indParams->apdu[2],
                        indParams->apdu[3], indParams->apdu[4], indParams->apdu[5],
                        indParams->apdu[6], indParams->apdu[7], indParams->apdu[8]);
            }
            else if (indParams->attr == AL_MSG_REQADDR_RESP)
            {
                /* Get parameters from APDU */
                /* Number of discovered nodes */
                numReportedNodes = indParams->apdu[0];
                /* If more than 4, only first 4 are included */
                if (numReportedNodes > 4)
                {
                    numReportedNodes = 4;
                }
                /* Print Info */
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "REQ ADDRESS RESPONSE Received. Discovered Nodes: %hhu\r\n", numReportedNodes);
                for (uint8_t i = 0; i < numReportedNodes; i++)
                {
                    rEntry = &appData.routingTable[appData.numFoundNodes];
                    /* First hops in route is the route node that discovered new Nodes */
                    (void) memcpy(rEntry->macAddress[0].address,
                        appData.routingTable[appData.numReqAddrSent].macAddress[0].address,
                        MAC_ADDRESS_SIZE * appData.routingTable[appData.numReqAddrSent].routeSize);
                    /* Last hop: ACA of Discovered Nodes, Store them in Routing table, but reversed to be used for addressing */
                    idx = (i * ADDR_RESP_INFO_LEN) + 1;
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[5] = indParams->apdu[idx + 0];
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[4] = indParams->apdu[idx + 1];
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[3] = indParams->apdu[idx + 2];
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[2] = indParams->apdu[idx + 3];
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[1] = indParams->apdu[idx + 4];
                    rEntry->macAddress[appData.routingTable[appData.numReqAddrSent].routeSize].address[0] = indParams->apdu[idx + 5];
                    rEntry->routeSize = appData.routingTable[appData.numReqAddrSent].routeSize + 1;
                    appData.numFoundNodes++;
                    appData.numFoundNodesNew++;
                    /* Print Info */
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "ACA=0x%02X%02X%02X%02X%02X%02X, Av_SIG=%hhu Av_SNR=%hhu, Av_TX=%hhu\r\n",
                            indParams->apdu[idx + 0], indParams->apdu[idx + 1], indParams->apdu[idx + 2],
                            indParams->apdu[idx + 3], indParams->apdu[idx + 4], indParams->apdu[idx + 5],
                            indParams->apdu[idx + 6], indParams->apdu[idx + 7], indParams->apdu[idx + 8]);
                }
            }
            else
            {
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Network Management command not handled by this APP\r\n");
            }
        }
    }
    else
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "DSAP not recognized\r\n");
    }

    /* Turn on indication LED and start timer to turn it off */
    SYS_TIME_TimerDestroy(appData.tmrPlcIndLedHandle);
    PLC_IND_LED_On();
    appData.tmrPlcIndLedHandle = SYS_TIME_CallbackRegisterMS(lAPP_PlcIndLedTimerCallback, 0, LED_PLC_RX_MSG_RATE_MS, SYS_TIME_SINGLE);
}

static void lAPP_AL_DataConfirmCallback(AL_DATA_CONFIRM_PARAMS *cfmParams)
{
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL_DATA_CONFIRM: Result=%hhu DSAP=0x%02X, ECC=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X\r\n",
            cfmParams->txStatus, cfmParams->dsap, cfmParams->ecc,
            cfmParams->dstAddress.address[0], cfmParams->dstAddress.address[1], cfmParams->dstAddress.address[2],
            cfmParams->dstAddress.address[3], cfmParams->dstAddress.address[4], cfmParams->dstAddress.address[5]);

    if (cfmParams->txStatus != AL_TX_STATUS_SUCCESS)
    {
        appData.state = APP_STATE_IDLE;
    }
}

static void lAPP_AL_EventIndicationCallback(AL_EVENT_IND_PARAMS *indParams)
{
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL_EVENT_INDICATION: Id=0x%02X, Value=0x", indParams->eventId);

    for (uint8_t i = 0; i < indParams->eventValue.length; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", indParams->eventValue.value[i]);
    }

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    if ((indParams->eventId == AL_EVENT_ID_MAC_ACA) || (indParams->eventId == AL_EVENT_ID_MASTER_TX_TIMEOUT))
    {
        if ((appData.state == APP_STATE_WAIT_ADDR_RESP) || (appData.state == APP_STATE_WAIT_REQADDR_RESP) ||
                (appData.state == APP_STATE_WAIT_TCT_SILENCE_NODES_SENT))
        {
            /* Master timeout expired. No more responses can be received. */
            appData.tmrStateTimeoutExpired = true;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static void lSendTCTBroadcast(uint8_t tctValue)
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Parameters related to TCT Set */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in TCT Set */
    reqParams.maxResponseLen = 20;
    reqParams.timeSlotNum = 0; /* Not used in TCT Set */
    reqParams.serviceClass = SERVICE_CLASS_S;
    reqParams.attr = AL_MSG_TCT_SET_REQ;
    reqParams.dsap = DLL_DSAP_NETWORK_MANAGEMENT;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: broadcast, no repeaters */
    reqParams.dstAddress.macAddress[0] = broadcastAddress;
    reqParams.dstAddress.routeSize = 1;
    /* Payload for TCT */
    txBuffer[0] = tctValue;
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 1;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND TCT BCAST: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendTCTSilence(uint8_t tctValue)
{
    AL_DATA_REQUEST_PARAMS reqParams;
    MAC_ADDRESS dstAddress;

    /* Parameters related to TCT Set */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in TCT Set */
    reqParams.maxResponseLen = 20;
    reqParams.timeSlotNum = 0; /* Not used in TCT Set */
    reqParams.serviceClass = SERVICE_CLASS_S;
    reqParams.attr = AL_MSG_TCT_SET_REQ;
    reqParams.dsap = DLL_DSAP_NETWORK_MANAGEMENT;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: Node in the routing table */
    (void) memcpy(reqParams.dstAddress.macAddress[0].address,
        appData.routingTable[appData.numTCTSilenceSent].macAddress[0].address,
        MAC_ADDRESS_SIZE * appData.routingTable[appData.numTCTSilenceSent].routeSize);
    reqParams.dstAddress.routeSize = appData.routingTable[appData.numTCTSilenceSent].routeSize;
    /* Payload for TCT */
    txBuffer[0] = tctValue;
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 1;

    dstAddress = reqParams.dstAddress.macAddress[reqParams.dstAddress.routeSize];
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND TCT SILENCE: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            dstAddress.address[0], dstAddress.address[1], dstAddress.address[2],
            dstAddress.address[3], dstAddress.address[4], dstAddress.address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendTCTBroadcastRepeater(uint8_t tctValue)
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Parameters related to TCT Set */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in TCT Set */
    reqParams.maxResponseLen = 20;
    reqParams.timeSlotNum = 0; /* Not used in TCT Set */
    reqParams.serviceClass = SERVICE_CLASS_S;
    reqParams.attr = AL_MSG_TCT_SET_REQ;
    reqParams.dsap = DLL_DSAP_NETWORK_MANAGEMENT;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: Node in the routing table */
    (void) memcpy(reqParams.dstAddress.macAddress[0].address,
        appData.routingTable[appData.numTCTBroadcastRepeaterSent].macAddress[0].address,
        MAC_ADDRESS_SIZE * appData.routingTable[appData.numTCTBroadcastRepeaterSent].routeSize);
    /* Last hop: broadcast address */
    reqParams.dstAddress.macAddress[appData.routingTable[appData.numTCTBroadcastRepeaterSent].routeSize] = broadcastAddress;
    reqParams.dstAddress.routeSize = appData.routingTable[appData.numTCTBroadcastRepeaterSent].routeSize + 1;
    /* Payload for TCT */
    txBuffer[0] = tctValue;
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 1;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND TCT BCAST: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendAddressReq()
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Parameters related to Address Req */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in Address Req */
    reqParams.maxResponseLen = 32;
    reqParams.timeSlotNum = 16;
    reqParams.serviceClass = SERVICE_CLASS_RC;
    reqParams.attr = AL_MSG_ADDRESS_REQ;
    reqParams.dsap = DLL_DSAP_NETWORK_MANAGEMENT;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: broadcast, no repeaters */
    reqParams.dstAddress.macAddress[0] = broadcastAddress;
    reqParams.dstAddress.routeSize = 1;
    /* Payload for Address Req */
    txBuffer[0] = 0x02; /* Respond in any Phase */
    txBuffer[1] = 0x55; /* TCR */
    txBuffer[2] = 0x00; /* All addresses respond */
    txBuffer[3] = 0x00; /* All addresses respond */
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 4;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND ADDRESS Request: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendReqAddressReq()
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Parameters related to Address Req */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in Address Req */
    reqParams.maxResponseLen = 32;
    reqParams.timeSlotNum = 16;
    reqParams.serviceClass = SERVICE_CLASS_RC;
    reqParams.attr = AL_MSG_REQADDR_REQ;
    reqParams.dsap = DLL_DSAP_NETWORK_MANAGEMENT;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: Node in the routing table */
    (void) memcpy(reqParams.dstAddress.macAddress[0].address,
        appData.routingTable[appData.numReqAddrSent].macAddress[0].address,
        MAC_ADDRESS_SIZE * appData.routingTable[appData.numReqAddrSent].routeSize);
    /* Last hop: broadcast address */
    reqParams.dstAddress.macAddress[appData.routingTable[appData.numReqAddrSent].routeSize] = broadcastAddress;
    reqParams.dstAddress.routeSize = appData.routingTable[appData.numReqAddrSent].routeSize + 1;
    /* Payload for Address Req */
    txBuffer[0] = 0x02; /* Respond in any Phase */
    txBuffer[1] = 0x55; /* TCR */
    txBuffer[2] = 0x00; /* All addresses respond */
    txBuffer[3] = 0x00; /* All addresses respond */
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 4;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND REQ ADDRESS Request: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendReadPlainReq()
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Parameters related to Address Req */
    reqParams.datetime = 0;
    reqParams.lmon = 0; /* Not used in Plain Read */
    reqParams.maxResponseLen = 32;
    reqParams.timeSlotNum = 0; /* Not used in Plain Read */
    reqParams.serviceClass = SERVICE_CLASS_RA;
    reqParams.attr = AL_MSG_READ_REQ;
    reqParams.dsap = DLL_DSAP_APPLICATION_FRAME;
    reqParams.ecc = DLL_ECC_DISABLED;
    /* Destination address: routing entry */
    (void) memcpy(&reqParams.dstAddress,
        &appData.routingTable[appData.numReadPlainSent],
        sizeof(ROUTING_ENTRY));
    /* Payload for Read Req (Read Status) */
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x3F;
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 2;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND READ Request: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

static void lSendReadEncryptedReq()
{
    AL_DATA_REQUEST_PARAMS reqParams;

    /* Set Auth Destination Node IB for Encryption */
    AL_IB_VALUE valueDestACA = {
        .length = 6,
        .value = {0x00}
    };
    /* Reverse ACA for Auth IB */
    valueDestACA.value[0] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[5];
    valueDestACA.value[1] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[4];
    valueDestACA.value[2] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[3];
    valueDestACA.value[3] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[2];
    valueDestACA.value[4] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[1];
    valueDestACA.value[5] = appData.routingTable[appData.numReadEncryptedSent].macAddress[0].address[0];
    AL_SetRequest(AL_AUTH_DESTINATION_NODE_ACA_IB, 0, &valueDestACA);

    /* Parameters related to Address Req */
    reqParams.datetime = 0;
    reqParams.lmon = appData.lmonTable[appData.numReadEncryptedSent];
    reqParams.maxResponseLen = 32;
    reqParams.timeSlotNum = 0; /* Not used in Plain Read */
    reqParams.serviceClass = SERVICE_CLASS_RA;
    reqParams.attr = AL_MSG_READ_REQ_AUTH;
    reqParams.dsap = DLL_DSAP_APPLICATION_FRAME;
    reqParams.ecc = DLL_ECC_AES_CTR_READ_KEY;
    /* Destination address: routing entry */
    (void) memcpy(&reqParams.dstAddress,
        &appData.routingTable[appData.numReadEncryptedSent],
        sizeof(ROUTING_ENTRY));
    /* Payload for Read Req (Read Status) */
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x3F;
    reqParams.apdu = txBuffer;
    reqParams.apduLen = 2;

    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SEND READ Request: DSAP=0x%02X, ECC=0x%02X, ATTR=0x%02X, DestAddr=0x%02X%02X%02X%02X%02X%02X",
            reqParams.dsap, reqParams.ecc, reqParams.attr,
            reqParams.dstAddress.macAddress[0].address[0], reqParams.dstAddress.macAddress[0].address[1], reqParams.dstAddress.macAddress[0].address[2],
            reqParams.dstAddress.macAddress[0].address[3], reqParams.dstAddress.macAddress[0].address[4], reqParams.dstAddress.macAddress[0].address[5]);
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, " APDU=0x");
    for (uint8_t i = 0; i < reqParams.apduLen; i++)
    {
        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", reqParams.apdu[i]);
    }
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");

    /* Send Data Request to AL */
    AL_DataRequest(&reqParams);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* IDLE state is used to signal when application is started */
    appData.state = APP_STATE_INIT;

    /* Initialize Timer variables */
    appData.tmrBlinkLedHandle = SYS_TIME_HANDLE_INVALID;
    appData.tmrPlcIndLedHandle = SYS_TIME_HANDLE_INVALID;
    appData.tmrStateTimeoutHandle = SYS_TIME_HANDLE_INVALID;
    appData.tmrBlinkLedExpired = false;
    appData.tmrPlcIndLedExpired = false;
    appData.tmrStateTimeoutExpired = false;
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    const uint8_t alK1[AL_KEY_LENGTH] = CONFIG_AL_KEY_K1;
    const uint8_t alK2[AL_KEY_LENGTH] = CONFIG_AL_KEY_K2;

    /* Refresh WDG */
    CLEAR_WATCHDOG();

    /* Blinking LED management */
    if (appData.tmrBlinkLedExpired)
    {
        appData.tmrBlinkLedExpired = false;
        BLINK_LED_Toggle();
    }

    /* PLC indication LED management */
    if (appData.tmrPlcIndLedExpired)
    {
        appData.tmrPlcIndLedExpired = false;
        PLC_IND_LED_Off();
    }

    /* Check the application's current state. */
    switch ( appData.state )
    {
        case APP_STATE_INIT:
        {
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "DCU Example App Started\r\n");
            if (appData.tmrBlinkLedHandle == SYS_TIME_HANDLE_INVALID)
            {
                /* Initialize Timer to handle blinking LED */
                appData.tmrBlinkLedHandle = SYS_TIME_CallbackRegisterMS(lAPP_BlinkLedTimerCallback, 0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);
            }

            /* Set AL callbacks */
            AL_DataIndicationCallbackRegister(lAPP_AL_DataIndicationCallback);
            AL_DataConfirmCallbackRegister(lAPP_AL_DataConfirmCallback);
            AL_EventIndicationCallbackRegister(lAPP_AL_EventIndicationCallback);

            /* Set AL Encryption Keys */
            appData.alIB.length = AL_KEY_LENGTH;
            (void) memcpy(appData.alIB.value, alK1, AL_KEY_LENGTH);
            AL_SetRequest(AL_AUTH_WRITE_KEY_K1_IB, 0, &appData.alIB);
            (void) memcpy(appData.alIB.value, alK2, AL_KEY_LENGTH);
            AL_SetRequest(AL_AUTH_READ_KEY_K2_IB, 0, &appData.alIB);

            appData.state = APP_STATE_WAIT_AL_READY;
            break;
        }

        case APP_STATE_WAIT_AL_READY:
        {
            if (AL_GetStatus() == SYS_STATUS_READY)
            {
                appData.state = APP_STATE_CLEAR_NODE_LIST;
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL ready\r\n");
            }

            break;
        }

        case APP_STATE_CLEAR_NODE_LIST:
        {
            /* Clear Routing Table */
            (void) memset(&appData.routingTable, 0, sizeof(appData.routingTable));
            /* Clear LMON Table */
            (void) memset(&appData.lmonTable, 0, sizeof(appData.lmonTable));
            /* Clear control variables */
            appData.numTCTBroadcastSent = 0;
            appData.numTCTSilenceSent = 0;
            appData.numTCTBroadcastRepeaterSent = 0;
            appData.numFoundNodes = 0;
            appData.numReqAddrSent = 0;
            appData.numReadPlainSent = 0;
            appData.numReadEncryptedSent = 0;
            appData.addressReqFinished = false;
            appData.lmonMismatchReceived = false;
            /* Set next State */
            appData.state = APP_STATE_SEND_TCT_BROADCAST;
            SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: SEND TCT BROADCAST\r\n");

            break;
        }

        case APP_STATE_SEND_TCT_BROADCAST:
        {
            lSendTCTBroadcast(MAX_TCT_VALUE);
            appData.state = APP_STATE_WAIT_TCT_SENT;
            /* Start Timer to wait for next state */
            SYS_TIME_TimerDestroy(appData.tmrStateTimeoutHandle);
            appData.tmrStateTimeoutHandle = SYS_TIME_CallbackRegisterMS(lAPP_StateTimerCallback, 0, STATE_TIMEOUT_MS, SYS_TIME_SINGLE);
            break;
        }

        case APP_STATE_WAIT_TCT_SENT:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                appData.numTCTBroadcastSent++;
                if (appData.numTCTBroadcastSent >= 2)
                {
                    /* Start looking for Nodes */
                    appData.state = APP_STATE_SEND_ADDR_REQ;
                    appData.numTCTBroadcastSent = 0;
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: DISCOVER NODES AT FIRST LEVEL\r\n");
                }
                else
                {
                    /* Send TCT Set again */
                    appData.state = APP_STATE_SEND_TCT_BROADCAST;
                }
            }
            break;
        }

        case APP_STATE_SEND_ADDR_REQ:
        {
            appData.numFoundNodesNew = 0;
            lSendAddressReq();
            appData.state = APP_STATE_WAIT_ADDR_RESP;
            break;
        }

        case APP_STATE_WAIT_ADDR_RESP:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                if (appData.numFoundNodes > 0)
                {
                    if (appData.numFoundNodesNew > 0)
                    {
                        /* Send TCT_SET to silence found nodes */
                        appData.state = APP_STATE_SEND_TCT_SILENCE_NODES;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: SEND TCT SILENCE DISCOVERED NODES\r\n");
                    }
                    else
                    {
                        /* Send TCT set broadcast through repeaters */
                        appData.state = APP_STATE_SEND_TCT_BROADCAST_REPEATER;
                        appData.addressReqFinished = true;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: SEND TCT BROADCAST TROUGH OTHER NODES\r\n");
                    }
                }
                else
                {
                    /* Try to find Nodes again */
                    appData.state = APP_STATE_SEND_ADDR_REQ;
                }
            }
            break;
        }

        case APP_STATE_SEND_TCT_SILENCE_NODES:
            lSendTCTSilence(MIN_TCT_VALUE);
            appData.state = APP_STATE_WAIT_TCT_SILENCE_NODES_SENT;
            break;

        case APP_STATE_WAIT_TCT_SILENCE_NODES_SENT:
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                appData.numTCTSilenceSent++;
                if (appData.numFoundNodes >= appData.numTCTSilenceSent)
                {
                    if (appData.addressReqFinished == false)
                    {
                        /* Look for nodes again */
                        appData.state = APP_STATE_SEND_ADDR_REQ;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: DISCOVER NODES AT FIRST LEVEL\r\n");
                    }
                    else
                    {
                        /* Look for Nodes through discovered Nodes */
                        appData.state = APP_STATE_SEND_REQADDR_REQ;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: DISCOVER NODES SEEN TROUGH OTHER NODES\r\n");
                    }
                }
                else
                {
                    /* Send new TCT to silence next node */
                    appData.state = APP_STATE_SEND_TCT_SILENCE_NODES;
                }
            }
            break;

        case APP_STATE_SEND_TCT_BROADCAST_REPEATER:
            lSendTCTBroadcastRepeater(MAX_TCT_VALUE);
            appData.state = APP_STATE_WAIT_TCT_SENT_REPEATER;
            /* Start Timer to wait for next state */
            SYS_TIME_TimerDestroy(appData.tmrStateTimeoutHandle);
            appData.tmrStateTimeoutHandle = SYS_TIME_CallbackRegisterMS(lAPP_StateTimerCallback, 0, STATE_TIMEOUT_MS, SYS_TIME_SINGLE);            
            break;

        case APP_STATE_WAIT_TCT_SENT_REPEATER:
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                appData.numTCTBroadcastSent++;
                if (appData.numTCTBroadcastSent >= 2)
                {
                    appData.numTCTBroadcastSent = 0;
                    if (appData.numFoundNodes >= appData.numTCTBroadcastRepeaterSent)
                    {
                        /* Silence discovered Nodes again */
                        appData.state = APP_STATE_SEND_TCT_SILENCE_NODES;
                        appData.numTCTSilenceSent = 0;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: SEND TCT SILENCE DISCOVERED NODES\r\n");
                    }
                    else
                    {
                        /* Send TCT Set through next repeater */
                        appData.state = APP_STATE_SEND_TCT_BROADCAST_REPEATER;
                    }
                }
                else
                {
                    /* Send TCT Set again */
                    appData.state = APP_STATE_SEND_TCT_BROADCAST_REPEATER;
                }
            }
            break;
            
        case APP_STATE_SEND_REQADDR_REQ:
        {
            appData.numFoundNodesNew = 0;
            lSendReqAddressReq();
            appData.state = APP_STATE_WAIT_REQADDR_RESP;
            break;
        }

        case APP_STATE_WAIT_REQADDR_RESP:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                appData.numReqAddrSent++;
                if (appData.numFoundNodesNew > 0)
                {
                    /* Send TCT set broadcast through repeaters */
                    appData.state = APP_STATE_SEND_TCT_BROADCAST_REPEATER;
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: SEND TCT BROADCAST TROUGH OTHER NODES\r\n");
                }
                else if (appData.numFoundNodes >= appData.numReqAddrSent)
                {
                    /* Start Reading Data from Nodes */
                    appData.state = APP_STATE_SEND_PLAIN_READ_REQ;
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: READ PLAIN DATA FROM DISCOVERED NODES\r\n");
                }
                else
                {
                    /* Keep discovering Nodes */
                    appData.state = APP_STATE_SEND_REQADDR_REQ;
                }
            }
            break;
        }

        case APP_STATE_SEND_PLAIN_READ_REQ:
        {
            lSendReadPlainReq();
            appData.state = APP_STATE_WAIT_PLAIN_READ_RESP;
            /* Start Timer to wait for next state */
            SYS_TIME_TimerDestroy(appData.tmrStateTimeoutHandle);
            appData.tmrStateTimeoutHandle = SYS_TIME_CallbackRegisterMS(lAPP_StateTimerCallback, 0, STATE_TIMEOUT_MS, SYS_TIME_SINGLE);
            break;
        }

        case APP_STATE_WAIT_PLAIN_READ_RESP:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                appData.numReadPlainSent++;
                if (appData.numFoundNodes >= appData.numReadPlainSent)
                {
                    /* Start Reading Encrypted Data from Nodes */
                    appData.state = APP_STATE_SEND_ENCRYPTED_READ_REQ;
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: READ ENCRYPTED DATA FROM DISCOVERED NODES\r\n");
                }
                else
                {
                    /* Keep Reading Plain Data */
                    appData.state = APP_STATE_SEND_PLAIN_READ_REQ;
                }
            }
            break;
        }

        case APP_STATE_SEND_ENCRYPTED_READ_REQ:
        {
            lSendReadEncryptedReq();
            appData.state = APP_STATE_WAIT_ENCRYPTED_READ_RESP;
            /* Start Timer to wait for next state */
            SYS_TIME_TimerDestroy(appData.tmrStateTimeoutHandle);
            appData.tmrStateTimeoutHandle = SYS_TIME_CallbackRegisterMS(lAPP_StateTimerCallback, 0, STATE_TIMEOUT_MS, SYS_TIME_SINGLE);
            break;
        }

        case APP_STATE_WAIT_ENCRYPTED_READ_RESP:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                appData.tmrStateTimeoutExpired = false;
                if (appData.lmonMismatchReceived == true)
                {
                    appData.lmonMismatchReceived = false;
                    /* Send again to same node */
                    appData.state = APP_STATE_SEND_ENCRYPTED_READ_REQ;
                }
                else
                {
                    appData.numReadEncryptedSent++;
                    if (appData.numFoundNodes >= appData.numReadEncryptedSent)
                    {
                        /* All Data Read, go to Clear Node List and start again */
                        appData.state = APP_STATE_CLEAR_NODE_LIST;
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\nNext App State: ALL DATA READ. CLEAR NODE LIST AND START OVER\r\n");
                    }
                    else
                    {
                        /* Keep Reading Encrypted Data */
                        appData.state = APP_STATE_SEND_ENCRYPTED_READ_REQ;
                    }
                }
            }
            break;
        }

        case APP_STATE_IDLE:
        {
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
