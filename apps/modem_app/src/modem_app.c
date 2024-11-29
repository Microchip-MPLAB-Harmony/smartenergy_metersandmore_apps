/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    modem_app.c

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

#include "definitions.h"
#include "modem_app.h"
#include "system/console/sys_console.h"
#include "stack/metersandmore/mmhi/mmhi_mib.h"
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define MODEM_APP_LIVE_LED_TIME_MS           250

static uint8_t modemAppMacDataTxBuffer[MAX_LENGTH_432_DATA];
static uint8_t modemAppMacDataRxBuffer[MAX_LENGTH_432_DATA];

const MMHI_MIB_ADDRESS modemAppMacAddress = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00}};

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the MODEM_APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

MODEM_APP_DATA modem_appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void lMODEM_APP_TimerCallback ( uintptr_t context )
{
    BSP_LED0_Toggle();
}

static void lMODEM_APP_MacDataReqCallback ( uint8_t dsap, uint8_t reqId, 
    uint8_t* lsdu, uint8_t lsduLen )
{
    AL_DATA_REQUEST_PARAMS_HI *drParams;
    uint8_t *pData = lsdu;
    uint8_t lt;
    
    modem_appData.reqId = reqId;
    drParams = &modem_appData.drParams;
    
    /* TBD: Parse the LSDU from MMHC */
    lt = *pData++;
    
    if (lsduLen < MAX_LENGTH_432_DATA)
    {
        memcpy(modemAppMacDataTxBuffer, lsdu, lsduLen);

        drParams->dsap = dsap;
        drParams->reqID = reqId;
        drParams->payload = modemAppMacDataTxBuffer;
        drParams->payloadLen = (uint16_t)lt;
        
        modem_appData.state = MODEM_APP_STATE_TX_FRAME;
    }
}

static void lMODEM_APP_AL_DataConfirm_callback(AL_DATA_CONFIRM_PARAMS *cfmParams)
{
    uint8_t command;
    uint8_t value = 0;
    
    if (cfmParams->txStatus == AL_TX_STATUS_SUCCESS)
    {
        command = modem_appData.master == true? MMHI_CMD_MASTER_DATA_CFM : MMHI_CMD_SLAVE_DATA_CFM;
    }
    else
    {
        command = modem_appData.master == true? MMHI_CMD_MASTER_DATA_NCFM : MMHI_CMD_SLAVE_DATA_NCFM;
    }
    
    MMHI_SendCommandFrame(command, &value, 1);
    
    SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, 
            "AL_DATA_CONFIRM: Result=%hhu DSAP=%hhu, ECC=%hhu, DestAddr=0x %02X%02X%02X%02X%02X%02X\r\n", 
            cfmParams->txStatus, cfmParams->dsap, cfmParams->ecc,
            cfmParams->dstAddress.address[0], cfmParams->dstAddress.address[1], 
            cfmParams->dstAddress.address[2], cfmParams->dstAddress.address[3], 
            cfmParams->dstAddress.address[4], cfmParams->dstAddress.address[5]);
}

static void lMODEM_APP_AL_DataIndication_callback(AL_DATA_IND_PARAMS *indParams)
{
    uint8_t *pData;
    MMHI_COMMAND command;
    
    if (modem_appData.master == true)
    {
        command = MMHI_CMD_MASTER_DATA_IND;
    }
    else
    {
        command = MMHI_CMD_SLAVE_DATA_IND;
    }
    
    pData = modemAppMacDataRxBuffer;
    *pData++ = modem_appData.drParams.dsap;
    *pData++ = modem_appData.reqId;
    
    /* TBD: Serialize the LSDU to send to MMHC */
    
    
    MMHI_SendCommandFrame(command, modemAppMacDataRxBuffer, pData - modemAppMacDataRxBuffer);
    
    SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, 
            "AL_DATA_INDICATION: DSAP=%hhu, ECC=%hhu, SrcAddr=0x %02X%02X%02X%02X%02X%02X, LSDU=0x", 
            indParams->dsap, indParams->ecc,
            indParams->srcAddress.address[0], indParams->srcAddress.address[1], 
            indParams->srcAddress.address[2], indParams->srcAddress.address[3], 
            indParams->srcAddress.address[4], indParams->srcAddress.address[5]);
}

static void lMODEM_APP_AL_EventIndication_callback(AL_EVENT_IND_PARAMS *indParams)
{
    SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, 
            "AL_EVENT_INDICATION: Id=%hhu, Value=0x", indParams->eventId);

    for (uint8_t i = 0; i < indParams->eventValue.length; i++)
    {
        SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "%02X", indParams->eventValue.value[i]);
    }

    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0,"\r\n");
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MODEM_APP_Initialize ( void )

  Remarks:
    See prototype in modem_app.h.
 */

void MODEM_APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    modem_appData.state = MODEM_APP_STATE_INIT;
    
    
    modem_appData.master = true;
    
}


/******************************************************************************
  Function:
    void MODEM_APP_Tasks ( void )

  Remarks:
    See prototype in modem_app.h.
 */

void MODEM_APP_Tasks ( void )
{
    USER_UPDATE_WDT();
    
    /* Check the application's current state. */
    switch ( modem_appData.state )
    {
        /* Application's initial state. */
        case MODEM_APP_STATE_INIT:
        {
            modem_appData.tmrHandle = SYS_TIME_CallbackRegisterMS(lMODEM_APP_TimerCallback, 
                    0, MODEM_APP_LIVE_LED_TIME_MS, SYS_TIME_PERIODIC);
            modem_appData.mmhiHandle = MMHI_Open((SYS_MODULE_OBJ)MMHI_INDEX);
            if (modem_appData.mmhiHandle != MMHI_HANDLE_INVALID)
            {
                MMHI_MacDataCallbackRegister(lMODEM_APP_MacDataReqCallback);
                
                AL_DataIndicationCallbackRegister(lMODEM_APP_AL_DataIndication_callback);
                AL_DataConfirmCallbackRegister(lMODEM_APP_AL_DataConfirm_callback);
                AL_EventIndicationCallbackRegister(lMODEM_APP_AL_EventIndication_callback);

                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Modem App initialized\r\n");
                
                modem_appData.alIb.length = 6;
                memcpy(modem_appData.alIb.value, modemAppMacAddress.value, 6);
                AL_SetRequest(AL_MAC_ACA_ADDRESS_IB, 0, &modem_appData.alIb);
                
                modem_appData.state = MODEM_APP_STATE_WAIT_DLL_READY;
            }
            break;
        }

        case MODEM_APP_STATE_WAIT_DLL_READY:
        {
            if (AL_GetStatus() == SYS_STATUS_READY)
            {
                modem_appData.state = MODEM_APP_STATE_WAITING;
            }
            break;
        }

        case MODEM_APP_STATE_WAITING:
        {

            break;
        }

        case MODEM_APP_STATE_TX_FRAME:
        {
            AL_DataRequestHI(&modem_appData.drParams);
            modem_appData.state = MODEM_APP_STATE_WAITING;
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
