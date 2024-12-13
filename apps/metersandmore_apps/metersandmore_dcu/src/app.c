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

#define MAX_TCT_VALUE   0xFF

APP_DATA appData;

static uint8_t txBuffer[16];

static const uint8_t broadcastAddress[MAC_ADDRESS_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03};

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
            if ((indParams->attr == AL_MSG_READ_REQ) || (indParams->attr == AL_MSG_READ_REQ_AUTH))
            {
                /* READ.REQ message */
                if ((indParams->apdu[0] == 0x00) && (indParams->apdu[1] == 0x3F))
                {
                    /* Read Status Word */
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Received READ.REQ Status Word\r\n");
                }
                else
                {
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "READ.REQ not recognized\r\n");
                }
            }
            else
            {
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Application Frame command not recognized\r\n");
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
                rEntry->macAddress->address[5] = indParams->apdu[0];
                rEntry->macAddress->address[4] = indParams->apdu[1];
                rEntry->macAddress->address[3] = indParams->apdu[2];
                rEntry->macAddress->address[2] = indParams->apdu[3];
                rEntry->macAddress->address[1] = indParams->apdu[4];
                rEntry->macAddress->address[0] = indParams->apdu[5];
                rEntry->routeSize = 1;
                appData.numFoundNodes++;
                /* Print Info */
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "ADDRESS RESPONSE Received\r\n");
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "ACA=0x%02X%02X%02X%02X%02X%02X, Av_SIG=%hhu Av_SNR=%hhu, Av_TX=%hhu\r\n",
                        indParams->apdu[0], indParams->apdu[1], indParams->apdu[2],
                        indParams->apdu[3], indParams->apdu[4], indParams->apdu[5],
                        indParams->apdu[6], indParams->apdu[7], indParams->apdu[8]);
            }
            else
            {
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "Application Frame command not recognized\r\n");
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
    (void) memcpy(reqParams.dstAddress.macAddress[0].address, broadcastAddress, MAC_ADDRESS_SIZE);
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
    (void) memcpy(reqParams.dstAddress.macAddress[0].address, broadcastAddress, MAC_ADDRESS_SIZE);
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
            memcpy(appData.alIB.value, alK1, AL_KEY_LENGTH);
            AL_SetRequest(AL_AUTH_WRITE_KEY_K1_IB, 0, &appData.alIB);
            memcpy(appData.alIB.value, alK2, AL_KEY_LENGTH);
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
            /* Clear control variables */
            appData.numTCTSent = 0;
            appData.numFoundNodes = 0;
            /* Set next State */
            appData.state = APP_STATE_SEND_TCT_BROADCAST;

            break;
        }

        case APP_STATE_SEND_TCT_BROADCAST:
        {
            lSendTCTBroadcast(MAX_TCT_VALUE);
            appData.numTCTSent++;
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
                if (appData.numTCTSent >= 2)
                {
                    appData.state = APP_STATE_SEND_ADDR_REQ;
                }
                else
                {
                    appData.state = APP_STATE_SEND_TCT_BROADCAST;
                }
            }
            break;
        }

        case APP_STATE_SEND_ADDR_REQ:
        {
            lSendAddressReq();
            appData.state = APP_STATE_WAIT_ADDR_RESP;
            /* Start Timer to wait for next state */
            SYS_TIME_TimerDestroy(appData.tmrStateTimeoutHandle);
            appData.tmrStateTimeoutHandle = SYS_TIME_CallbackRegisterMS(lAPP_StateTimerCallback, 0, STATE_TIMEOUT_MS, SYS_TIME_SINGLE);
            break;
        }

        case APP_STATE_WAIT_ADDR_RESP:
        {
            if (appData.tmrStateTimeoutExpired)
            {
                if (appData.numFoundNodes > 0)
                {
                    appData.state = APP_STATE_SEND_REQADDR_REQ;
                }
                else
                {
                    appData.state = APP_STATE_SEND_ADDR_REQ;
                }

            }
            break;
        }

        case APP_STATE_SEND_REQADDR_REQ:
        {
            break;
        }

        case APP_STATE_WAIT_REQADDR_RESP:
        {
            break;
        }

        case APP_STATE_SEND_PLAIN_READ_REQ:
        {
            break;
        }

        case APP_STATE_WAIT_PLAIN_READ_RESP:
        {
            break;
        }

        case APP_STATE_SEND_ENCRYPTED_READ_REQ:
        {
            break;
        }

        case APP_STATE_WAIT_ENCRYPTED_READ_RESP:
        {
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
