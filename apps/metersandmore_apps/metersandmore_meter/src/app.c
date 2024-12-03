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

APP_DATA appData;

/* READ.RESP (003) message corresponding to Status Word (all flags set to 0) */
static const uint8_t appStatusWordRespMsg[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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

static void lAPP_AL_DataIndicationCallback(AL_DATA_IND_PARAMS *indParams)
{
    AL_DATA_REQUEST_PARAMS drParams;

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

    drParams.ecc = indParams->ecc;
    if (indParams->dsap == DLL_DSAP_APPLICATION_FRAME)
    {
        drParams.dsap = DLL_DSAP_APPLICATION_FRAME;
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

                    /* Send READ.RESP */
                    drParams.apdu = (uint8_t *)appStatusWordRespMsg;
                    drParams.apduLen = sizeof(appStatusWordRespMsg);
                    if (indParams->attr == AL_MSG_READ_REQ)
                    {
                        drParams.attr = AL_MSG_READ_RESP;
                    }
                    else
                    {
                        drParams.attr = AL_MSG_READ_RESP_AUTH;
                    }
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL_DATA_REQUEST (READ.RESP Status Word) Attr=%hhu, APDU=0x", drParams.attr);
                    for (uint8_t i = 0; i < drParams.apduLen; i++)
                    {
                        SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "%02X", drParams.apdu[i]);
                    }
                    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "\r\n");
                    AL_DataRequest(&drParams);
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

#ifdef CONFIG_ACA_AUTO_PIC32CXMT
static void lAPP_STORAGE_GetACA(uint8_t* aca)
{
    uint8_t uniqueId[16];

    /* Read UniqueID to set ACA (MAC Address) */
    SEFC0_UniqueIdentifierRead((uint32_t*) uniqueId, 4);
    aca[5] = uniqueId[6];
    aca[4] = uniqueId[7];
    aca[3] = (uniqueId[8] << 4) | (uniqueId[9] & 0x0F);
    aca[2] = (uniqueId[10] << 4) | (uniqueId[11] & 0x0F);
    aca[1] = (uniqueId[12] << 4) | (uniqueId[13] & 0x0F);
    aca[0] = (uniqueId[14] << 4) | (uniqueId[15] & 0x0F);
}
#endif

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
    appData.tmrBlinkLedExpired = false;
    appData.tmrPlcIndLedExpired = false;
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
#ifdef CONFIG_ACA_FIXED
    const uint8_t acaFixed[MAC_ADDRESS_SIZE] = CONFIG_ACA_FIXED;
#endif

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

            /* Set MAC address */
            appData.alIB.length = MAC_ADDRESS_SIZE;
#ifdef CONFIG_ACA_FIXED
            memcpy(appData.alIB.value, acaFixed, MAC_ADDRESS_SIZE);
#elif defined(CONFIG_ACA_AUTO_PIC32CXMT)
            lAPP_STORAGE_GetACA(appData.alIB.value);
#endif
            AL_SetRequest(AL_MAC_ACA_ADDRESS_IB, 0, &appData.alIB);

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
                appData.state = APP_STATE_IDLE;
                SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "AL ready\r\n");
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
