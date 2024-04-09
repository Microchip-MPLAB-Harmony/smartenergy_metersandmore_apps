/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_plc_pl460.c

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
#define DRV_PLC_PHY_INDEX_0   0
#define APP_PLC_CONFIG_KEY  0xA55A

/* PLC Driver Initialization Data (initialization.c) */
extern DRV_PLC_PHY_INIT drvPlcPhyInitData;

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

APP_PLC_DATA appPlc;
APP_PLC_DATA_TX appPlcTx;

static CACHE_ALIGN uint8_t appPlcPibDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_PIB_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t appPlcTxDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_BUFFER_SIZE)];

static const char appPlcCalibrationData[] = "Microchip, Enabling Unlimited Possibilities";
static APP_PLC_CALIBRATION_DATA appPlcCalibration;

#define DIV_ROUND(a, b)      (((a) + ((b) >> 1)) / (b))

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void APP_PLC_Timer1_Callback (uintptr_t context)
{
    appPlc.tmr1Expired = true;
}

void APP_PLC_Timer2_Callback (uintptr_t context)
{
    appPlc.tmr2Expired = true;
}

static void APP_PLC_ExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    /* Update PLC TX Status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
    /* Restore TX configuration */
    appPlc.state = APP_PLC_STATE_READ_CONFIG;
}

static void APP_PLC_PrintTxMessage(void)
{
    char *pBuffer = (char *)appPlcPibDataBuffer; // Use PIB Data buffer to minimize the static memory in use
    uint8_t *pData = appPlcTx.plcPhyTx.pTransmitData;
    uint16_t index;
    
    for(index = 0; index < appPlcTx.plcPhyTx.dataLength; index++)
    {
        sprintf(pBuffer, "%02X", *pData);
        pData++;
        pBuffer += 2;
    }
    *pBuffer = '\0';
    APP_CONSOLE_Print("\r\n%06d: 0x%s\r\n", appPlcTx.txNumSequence, appPlcPibDataBuffer);
}

static void APP_PLC_DataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;
    
    /* Update calibration variables */
    if (appPlc.state == APP_PLC_STATE_TX_CALIBRATION)
    {
        if (appPlcCalibration.rmsmaxCalibration == true)
        {
            appPlcCalibration.pValues[appPlcTx.plcPhyTx.attenuation] += cfmObj->rmsCalc;
        }
        else
        {
            DRV_PLC_PHY_PIB_OBJ pibObj;
            
            uint32_t correctedRmsCalc;
    
            /* Get Corrected RMS CALC */
            pibObj.id = PLC_ID_CORRECTED_RMS_CALC;
            pibObj.length = 4;
            pibObj.pData = (uint8_t *)&correctedRmsCalc;
            DRV_PLC_PHY_PIBGet(appPlc.drvPlcHandle, &pibObj);
    
            appPlcCalibration.pValues[appPlcTx.plcPhyTx.attenuation] += correctedRmsCalc;
        }
        
        appPlcCalibration.frameCounter++;
        
        if (appPlcCalibration.framesPerIteration == 0)
        {
            uint32_t rmsMean;
            rmsMean = DIV_ROUND(appPlcCalibration.pValues[appPlcTx.plcPhyTx.attenuation], APP_PLC_CALIBRATE_FRAMES_ITERATION);

            if (appPlcCalibration.rmsmaxCalibration == false)
            {
                /* Apply percentage depending on impedance mode */
                if (appPlcCalibration.impedanceCalibration == HI_STATE)
                {
                    rmsMean = DIV_ROUND(rmsMean * APP_PLC_CALIBRATE_THRESHOLD_HI_PERC, 100);
                }
                else
                {
                    rmsMean = DIV_ROUND(rmsMean * APP_PLC_CALIBRATE_THRESHOLD_VLO_PERC, 100);
                }
            }

            appPlcCalibration.pValues[appPlcTx.plcPhyTx.attenuation] = rmsMean;
        }
        
        if (cfmObj->result == DRV_PLC_PHY_TX_RESULT_SUCCESS)
        {
            APP_CONSOLE_Print(".");
            if (appPlcCalibration.frameCounter % 50 == 0)
            {
                APP_CONSOLE_Print("\r\n");
            }
        }
        else
        {
            APP_CONSOLE_Print("x");
        }

        appPlcCalibration.txConfirm = true;
    }
    else
    {
        /* Update PLC TX Status */
        appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;

        appPlcTx.plcPhyTx.timeIni = cfmObj->timeEnd + appPlcTx.timeBetweenFrames;
        appPlcTx.plcPhyTx.mode = TX_MODE_ABSOLUTE;

        /* Handle result of transmission : Show it through Console */
        switch(cfmObj->result)
        {
            case DRV_PLC_PHY_TX_RESULT_PROCESS:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_PROCESS\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_SUCCESS:
                /* Show TX message through Serial Console */
                APP_PLC_PrintTxMessage();
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_SUCCESS RMS_CALC %u\r\n", cfmObj->rmsCalc);
                break;
            case DRV_PLC_PHY_TX_RESULT_INV_LENGTH:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_INV_LENGTH\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_BUSY_CH\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_BUSY_TX:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_BUSY_TX\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_BUSY_RX\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_TIMEOUT:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_TIMEOUT\r\n");
                break;
            case DRV_PLC_PHY_TX_CANCELLED:
                APP_CONSOLE_Print("\r\n...DRV_PLC_PHY_TX_CANCELLED\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_120:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_120\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_110:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_110\r\n");
                break;
            case DRV_PLC_PHY_TX_RESULT_NO_TX:
                APP_CONSOLE_Print("...DRV_PLC_PHY_TX_RESULT_NO_TX. Press 'x' and review PVDD source.\r\n");
                appPlc.state = APP_PLC_STATE_STOP_TX;
                break;
        }
    }
}

static void APP_PLC_DataIndCb( DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    if (indObj->dataLength)
    {
        /* Turn on indication LED and start timer to turn it off */
        SYS_TIME_TimerDestroy(appPlc.tmr2Handle);
        USER_PLC_IND_LED_On();
        /* Start signal timer */
        appPlc.tmr2Handle = SYS_TIME_CallbackRegisterMS(APP_PLC_Timer2_Callback, 0, LED_PLC_RX_MSG_RATE_MS, SYS_TIME_SINGLE);
        
        /* Launch Data Indication callback */
        if (appPlc.dataIndCallback)
        {
            appPlc.dataIndCallback(indObj, 0);
        }
    }
}

#ifndef APP_PLC_DISABLE_PVDDMON
static void APP_PLC_PVDDMonitorCb( SRV_PVDDMON_CMP_MODE cmpMode, uintptr_t context )
{
    (void)context;
    
    if (!appPlc.pvddMonInitialized)
    {
        appPlc.pvddMonInitialized = true;
    }

    if (cmpMode == SRV_PVDDMON_CMP_MODE_OUT)
    {
        /* PLC Transmission is not permitted */
        DRV_PLC_PHY_EnableTX(appPlc.drvPlcHandle, false);
        appPlc.pvddMonTxEnable = false;
        /* Restart PVDD Monitor to check when VDD is within the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_IN);
    }
    else
    {
        /* PLC Transmission is permitted again */
        DRV_PLC_PHY_EnableTX(appPlc.drvPlcHandle, true);
        appPlc.pvddMonTxEnable = true;
        /* Restart PVDD Monitor to check when VDD is out of the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_OUT);
    }
}
#endif

/*******************************************************************************
  Function:
    void APP_PLC_PL460_Initialize(void)

  Remarks:
    See prototype in app_plc.h.
 */
void APP_PLC_PL460_Initialize ( void )
{
    /* IDLE state is used to signal when application is started */
    appPlc.state = APP_PLC_STATE_IDLE;

    /* Init PLC PIB buffer */
    appPlc.plcPIB.pData = appPlcPibDataBuffer;

    /* Init PLC TX Buffer */
    appPlcTx.plcPhyTx.pTransmitData = appPlcTxDataBuffer;

    /* Init Timer handler */
    appPlc.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr1Expired = false;
    appPlc.tmr2Expired = false;

    /* Init signalling */
    appPlc.signalResetCounter = LED_RESET_BLINK_COUNTER;

    /* Init PLC TX status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
    
    /* Init Data Indication Callback */
    appPlc.dataIndCallback = NULL;
    
}

/******************************************************************************
  Function:
    void APP_PLC_PL460_Tasks ( void )

  Remarks:
    See prototype in app_plc.h.
 */

void APP_PLC_PL460_Tasks ( void )
{
    /* Signalling: LED Toggle */
    if (appPlc.tmr1Expired)
    {
        appPlc.tmr1Expired = false;
        USER_BLINK_LED_Toggle();

        if (appPlc.signalResetCounter)
        {
            appPlc.signalResetCounter--;
        }
    }

    /* Signalling: PLC RX */
    if (appPlc.tmr2Expired)
    {
        appPlc.tmr2Expired = false;
        USER_PLC_IND_LED_Off();
    }

    /* Check the application's current state. */
    switch ( appPlc.state )
    {
        case APP_PLC_STATE_IDLE:
        {
            /* Signalling when the application is starting */
            if (appPlc.signalResetCounter)
            {
                if (appPlc.tmr1Handle == SYS_TIME_HANDLE_INVALID)
                {
                    /* Init Timer to handle blinking led */
                    appPlc.tmr1Handle = SYS_TIME_CallbackRegisterMS(APP_PLC_Timer1_Callback, 0, LED_RESET_BLINK_RATE_MS, SYS_TIME_PERIODIC);
                }
            }
            else
            {
                SYS_TIME_TimerDestroy(appPlc.tmr1Handle);
                appPlc.tmr1Handle = SYS_TIME_HANDLE_INVALID;

                /* Read configuration from NVM memory */
                appPlc.state = APP_PLC_STATE_READ_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_READ_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                appNvm.pData = (uint8_t*)&appPlcTx;
                appNvm.dataLength = sizeof(appPlcTx);
                appNvm.state = APP_NVM_STATE_READ_MEMORY;

                appPlc.state = APP_PLC_STATE_CHECK_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_CHECK_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                uint8_t* pData;
                uint16_t length;
                uint8_t dataValue;
                
                if (appPlcTx.configKey != APP_PLC_CONFIG_KEY)
                {
                    /* Set configuration by default */
                    appPlcTx.configKey = APP_PLC_CONFIG_KEY;
                    appPlcTx.plcPhyVersion = 0;
                    appPlcTx.plcPhyTx.timeIni = 0;
                    appPlcTx.plcPhyTx.attenuation = 0;
                    appPlcTx.plcPhyTx.mode = TX_MODE_RELATIVE;
                    appPlcTx.plcPhyTx.nbFrame = 0;
                    appPlcTx.plcPhyTx.dataLength = 64;
                    appPlcTx.plcPhyTx.pTransmitData = appPlcTxDataBuffer;
                    
                    appPlcTx.txAuto = 0;
                    appPlcTx.txImpedance = HI_STATE;
                    appPlcTx.timeBetweenFrames = 1000000;
                    
                    /* Clear Transmission flag */
                    appPlcTx.inTx = false;
                }
                
                /* Initialize data buffer data */
                dataValue = 0x30;
                length = APP_PLC_BUFFER_SIZE;
                pData = appPlcTxDataBuffer;
                while(length--)
                {
                    *pData++ = (uint8_t)dataValue++;
                    if (dataValue == 0x3A)
                    {
                        dataValue = 0x30;
                    }
                }

                /* Initialize PLC driver */
                appPlc.state = APP_PLC_STATE_INIT;
            }
            break;
        }

        case APP_PLC_STATE_WRITE_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                appNvm.pData = (uint8_t*)&appPlcTx;
                appNvm.dataLength = sizeof(appPlcTx);
                appNvm.state = APP_NVM_STATE_WRITE_MEMORY;

                appPlc.state = APP_PLC_STATE_WAIT_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_WAIT_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                if (appPlcTx.inTx)
                {
                    appPlc.state = APP_PLC_STATE_TX;
                }
                else
                {
                    appPlc.state = APP_PLC_STATE_WAITING;
                }
            }
            break;
        }

        case APP_PLC_STATE_INIT:
        {
            /* Open PLC driver */
            appPlc.drvPlcHandle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX_0, NULL);

            if (appPlc.drvPlcHandle != DRV_HANDLE_INVALID)
            {
                appPlc.state = APP_PLC_STATE_OPEN;
            }
            else
            {
                appPlc.state = APP_PLC_STATE_ERROR;
            }
            break;
        }

        case APP_PLC_STATE_OPEN:
        {
            /* Check PLC transceiver */
            if (DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX_0) == SYS_STATUS_READY)
            {
                DRV_PLC_PHY_PIB_OBJ pibObj;
                uint32_t version;

                /* Configure PLC callbacks */
                DRV_PLC_PHY_ExceptionCallbackRegister(appPlc.drvPlcHandle, APP_PLC_ExceptionCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_TxCfmCallbackRegister(appPlc.drvPlcHandle, APP_PLC_DataCfmCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_DataIndCallbackRegister(appPlc.drvPlcHandle, APP_PLC_DataIndCb, DRV_PLC_PHY_INDEX_0);

                /* Apply PLC coupling configuration */
                SRV_PCOUP_Set_Config(appPlc.drvPlcHandle, SRV_PLC_PCOUP_MAIN_BRANCH);
                
                /* Apply TX configuration */
                APP_PLC_SetImpedanceState(appPlcTx.txAuto, appPlcTx.txImpedance);
				
#ifndef APP_PLC_DISABLE_PVDDMON
                /* Disable TX Enable at the beginning */
                DRV_PLC_PHY_EnableTX(appPlc.drvPlcHandle, false);
                appPlc.pvddMonInitialized = false;
                appPlc.pvddMonTxEnable = false;
                /* Enable PLC PVDD Monitor Service */
                SRV_PVDDMON_CallbackRegister(APP_PLC_PVDDMonitorCb, 0);
                SRV_PVDDMON_Start(SRV_PVDDMON_CMP_MODE_IN);
#else
                /* Enable TX Enable at the beginning */
                DRV_PLC_PHY_EnableTX(appPlc.drvPlcHandle, true);
                appPlc.pvddMonTxEnable = true;
#endif

                /* Init Timer to handle blinking led */
                appPlc.tmr1Handle = SYS_TIME_CallbackRegisterMS(APP_PLC_Timer1_Callback, 0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);

                /* Get PLC PHY version */
                pibObj.id = PLC_ID_VERSION_NUM;
                pibObj.length = 4;
                pibObj.pData = (uint8_t *)&version;
                DRV_PLC_PHY_PIBGet(appPlc.drvPlcHandle, &pibObj);

                if (version == appPlcTx.plcPhyVersion)
                {
                    if (appPlcTx.inTx)
                    {
                        /* Previous Transmission state */
                        appPlc.state = APP_PLC_STATE_TX;
                    }
                    else
                    {
                        /* Nothing To Do */
                        appPlc.state = APP_PLC_STATE_WAITING;
                    }
                }
                else
                {
                    appPlcTx.plcPhyVersion = version;

                    /* Store configuration in NVM memory */
                    appPlc.state = APP_PLC_STATE_WRITE_CONFIG;
                }
            }
            break;
        }

        case APP_PLC_STATE_WAITING:
        {
            break;
        }

        case APP_PLC_STATE_TX:
        {
            if (!appPlcTx.inTx)
            {
                /* Set Transmission Mode */
                appPlcTx.plcPhyTx.mode = TX_MODE_RELATIVE;
                appPlcTx.plcPhyTx.timeIni = 0;

                /* Set Transmission flag */
                appPlcTx.inTx = true;

                /* Store TX configuration */
                appPlc.state = APP_PLC_STATE_WRITE_CONFIG;
            }
            else
            {
                if (appPlc.plcTxState == APP_PLC_TX_STATE_IDLE)
                {
                    if (appPlc.pvddMonInitialized)
                    {
                        if (appPlc.pvddMonTxEnable)
                        {
                            /* Update the sequence number */
                            appPlcTx.txNumSequence++;

                            appPlc.plcTxState = APP_PLC_TX_STATE_WAIT_TX_CFM;
                            /* Send PLC message */
                            DRV_PLC_PHY_TxRequest(appPlc.drvPlcHandle, &appPlcTx.plcPhyTx);
                        }
                        else
                        {
                            DRV_PLC_PHY_TRANSMISSION_CFM_OBJ cfmData;

                            cfmData.timeEnd = 0;
                            cfmData.rmsCalc = 0;
                            cfmData.result = DRV_PLC_PHY_TX_RESULT_NO_TX;
                            APP_PLC_DataCfmCb(&cfmData, 0);
                        }
                    }
                }
            }
            break;
        }

        case APP_PLC_STATE_STOP_TX:
        {
            /* Clear Transmission flag */
            appPlcTx.inTx = false;

            /* Store TX configuration */
            appPlc.state = APP_PLC_STATE_WRITE_CONFIG;

            /* Cancel last transmission */
            if (appPlc.plcTxState == APP_PLC_TX_STATE_WAIT_TX_CFM)
            {
                /* Send PLC Cancel message */
                appPlcTx.plcPhyTx.mode = TX_MODE_CANCEL;
                DRV_PLC_PHY_TxRequest(appPlc.drvPlcHandle, &appPlcTx.plcPhyTx);
            }
            break;
        }

        case APP_PLC_STATE_TX_CALIBRATION:
        {
            if (appPlcCalibration.txConfirm)
            {
                if (appPlcCalibration.framesPerIteration > 0)
                {
                    appPlcCalibration.txConfirm = false;
                    appPlcCalibration.framesPerIteration--;
                    DRV_PLC_PHY_TxRequest(appPlc.drvPlcHandle, &appPlcTx.plcPhyTx);
                }
                else
                {
                    appPlcTx.plcPhyTx.attenuation++;
                    if (appPlcTx.plcPhyTx.attenuation < SRV_PCOUP_NUM_TX_LEVELS)
                    {
                        appPlcCalibration.framesPerIteration = APP_PLC_CALIBRATE_FRAMES_ITERATION;
                    }
                    else
                    {
                        /* Calibration completed */
                        appPlcCalibration.valuesAreReady = true;
                        /* Restore Phy Tx configuration */
                        memcpy(&appPlcTx.plcPhyTx, &appPlcCalibration.plcPhyTxBackup, sizeof(appPlcTx.plcPhyTx));
                        APP_PLC_SetImpedanceState(appPlcTx.txAuto, appPlcTx.txImpedance);

                        appPlc.state = APP_PLC_STATE_WAITING;

                        /* Apply calibrated values */
                        DRV_PLC_PHY_PIB_OBJ pibObj;

                        if (appPlcCalibration.rmsmaxCalibration == true)
                        {
                            pibObj.length = 8 << 2;
                            pibObj.pData = (uint8_t *)appPlcCalibration.pValues;
                            if (appPlcCalibration.impedanceCalibration == HI_STATE)
                            {
                                pibObj.id = PLC_ID_MAX_RMS_TABLE_HI;
                            }
                            else
                            {
                                pibObj.id = PLC_ID_MAX_RMS_TABLE_VLO;
                            }
                        }
                        else
                        {
                            uint32_t thresholds[16];

                            pibObj.length = 8 << 3;
                            pibObj.pData = (uint8_t *)thresholds;
                            memset(thresholds, 0, sizeof(thresholds));
                            memcpy(&thresholds[8], appPlcCalibration.pValues, SRV_PCOUP_NUM_TX_LEVELS << 2);
                            if (appPlcCalibration.impedanceCalibration == HI_STATE)
                            {
                                pibObj.id = PLC_ID_THRESHOLDS_TABLE_HI;
                            }
                            else
                            {
                                pibObj.id = PLC_ID_THRESHOLDS_TABLE_VLO;
                            }
                        }

                        DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);
                    }
                }
            }
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            break;
        }
    }
}

void APP_PLC_DataIndCallbackRegister( const DRV_PLC_PHY_DATA_IND_CALLBACK callback )
{
    appPlc.dataIndCallback = callback;
}

void APP_PLC_SetImpedanceState(bool txAuto, uint8_t impedance)
{
    DRV_PLC_PHY_PIB_OBJ pibObj;
    
    appPlcTx.txAuto = (uint8_t)txAuto;
    appPlcTx.txImpedance = impedance;
    
    /* Set Autodetect Mode */
    pibObj.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
    pibObj.length = 1;
    pibObj.pData = (uint8_t *)&appPlcTx.txAuto;
    DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);

    /* Set Impedance Mode */
    pibObj.id = PLC_ID_CFG_IMPEDANCE;
    pibObj.length = 1;
    pibObj.pData = (uint8_t *)&appPlcTx.txImpedance;
    DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);
}

void APP_PLC_GetCalibrationValues(uint8_t type, uint8_t impedance)
{
    DRV_PLC_PHY_PIB_OBJ pibObj;
    uint8_t value;
    
    /* Configure Autodetect Mode depending on calibration type */
    if (type == APP_PLC_CALIBRATE_RMSMAX)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    pibObj.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
    pibObj.length = 1;
    pibObj.pData = (uint8_t *)&value;
    DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);

    /* Set Impedance Mode */
    pibObj.id = PLC_ID_CFG_IMPEDANCE;
    pibObj.length = 1;
    pibObj.pData = (uint8_t *)&impedance;
    DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);
    
    /* Store Phy Tx current configuration */
    memcpy(&appPlcCalibration.plcPhyTxBackup, &appPlcTx.plcPhyTx, sizeof(appPlcTx.plcPhyTx));
    
    /* Apply new TX configuration for calibration */
    appPlcTx.plcPhyTx.timeIni = 0;
    appPlcTx.plcPhyTx.attenuation = 0;
    appPlcTx.plcPhyTx.mode = TX_MODE_RELATIVE;
    appPlcTx.plcPhyTx.nbFrame = 0;
    appPlcTx.plcPhyTx.dataLength = sizeof(appPlcCalibrationData);
    appPlcTx.plcPhyTx.pTransmitData = (uint8_t *)appPlcCalibrationData;
    
    /* Set Calibration configuration */
    appPlcCalibration.valuesAreReady = false;
    appPlcCalibration.txConfirm = true;
    appPlcCalibration.framesPerIteration = APP_PLC_CALIBRATE_FRAMES_ITERATION;
    appPlcCalibration.impedanceCalibration = impedance;

    if (type == APP_PLC_CALIBRATE_RMSMAX)
    {
        appPlcCalibration.rmsmaxCalibration = true;
        if (impedance == HI_STATE)
        {
            appPlcCalibration.pValues = appPlcCalibration.rmsMaxValuesHi;
            appPlcCalibration.rmsmaxCalibration = true;
        }
        else
        {
            appPlcCalibration.pValues = appPlcCalibration.rmsMaxValuesVlo;
        }
    }
    else
    {
        uint32_t thresholds[16];

        appPlcCalibration.rmsmaxCalibration = false;
        if (impedance == HI_STATE)
        {
            appPlcCalibration.pValues = appPlcCalibration.thresholdValuesHi;
            pibObj.id = PLC_ID_THRESHOLDS_TABLE_HI;
            memset(thresholds, 0, sizeof(thresholds));
        }
        else
        {
            appPlcCalibration.pValues = appPlcCalibration.thresholdValuesVlo;
            pibObj.id = PLC_ID_THRESHOLDS_TABLE_VLO;
            memset(thresholds, 0xFF, sizeof(thresholds));
        }

        /* Disable thresholds */
        pibObj.length = sizeof(thresholds);
        pibObj.pData = (uint8_t *)&thresholds;
        DRV_PLC_PHY_PIBSet(appPlc.drvPlcHandle, &pibObj);
    }
    
    memset(appPlcCalibration.pValues, 0, sizeof(appPlcCalibration.rmsMaxValuesHi));
    appPlcCalibration.frameCounter = 0;
                
    /* Start TX for calibration */
    appPlc.state = APP_PLC_STATE_TX_CALIBRATION;
    
}

bool APP_PLC_CalibrationValuesAreReady(uint32_t **pValues)
{
    if (appPlcCalibration.valuesAreReady)
    {
        *pValues = appPlcCalibration.pValues;
        return true;
    }
    
    return false;
}

void APP_PLC_StartTramission(void)
{
    appPlcTx.txNumSequence = 0;
    appPlc.state = APP_PLC_STATE_TX;
}

/*******************************************************************************
 End of File
 */
