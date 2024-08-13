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
    app_console.c

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
#include <stdarg.h>
#include <math.h>
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


static va_list sArgs = {0};

/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

CACHE_ALIGN APP_CONSOLE_DATA appConsole;

static CACHE_ALIGN char pTransmitBuffer[CACHE_ALIGNED_SIZE_GET(SERIAL_BUFFER_SIZE)];
static CACHE_ALIGN char pReceivedBuffer[CACHE_ALIGNED_SIZE_GET(SERIAL_BUFFER_SIZE)];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static void APP_CONSOLE_PLCDataIndicationCallback(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context)
{
    uint8_t *pData = indObj->pReceivedData;
    uint16_t index;
    
    appConsole.rxNumSequence++;

    APP_CONSOLE_Print("\r\n%06u: 0x", appConsole.rxNumSequence);
    
    for(index = 0; index < indObj->dataLength; index++)
    {
        APP_CONSOLE_Print("%02X", *pData);
        pData++;
    }

    APP_CONSOLE_Print("\r\n  Len:%hu CRC_Ok:%hhu RSSI:%hhu dBuV LQI:%hhu (%2.2f dB) SNR_H:%2.2f dB SNR_P:%2.2f dB NBrx:%hhu Duration:%u us Delta:%d us\r\n",
            indObj->dataLength, indObj->crcOk, indObj->rssi, indObj->lqi, (indObj->lqi / 4.0f) - 10.0f, indObj->snrHeader / 4.0f,
                      indObj->snrPayload / 4.0f, indObj->nbRx, indObj->frameDuration, (int32_t)(indObj->timeEnd - indObj->frameDuration) - (int32_t)appConsole.rxTimeEndPrev);

    /* Update Rx Time End to calculate delta in next reception */
    appConsole.rxTimeEndPrev = indObj->timeEnd;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static bool APP_CONSOLE_CheckIsPrintable(char data)
{
    if (((data >= 32) && (data <= 126)) ||
        ((data >= 128) && (data <= 254)) ||
         (data == '\t') || (data == '\n'))
    {
        return 1;
    }

    return 0;
}

static void APP_CONSOLE_ReadRestart( uint8_t numCharPending )
{
    appConsole.pNextChar = appConsole.pReceivedChar;
    appConsole.dataLength = 0;
    appConsole.numCharToReceive = numCharPending;
}

static uint8_t APP_CONSOLE_ReadSerialChar( void )
{
    if (appConsole.numCharToReceive > 0)
    {
        if (SYS_CONSOLE_Read(SYS_CONSOLE_INDEX_0, (void*)appConsole.pNextChar, 1) > 0)
        {
            /* Success */
            if ((*(appConsole.pNextChar) == '\r') || (*(appConsole.pNextChar) == '\n'))
            {
                appConsole.numCharToReceive = 0;
                return appConsole.dataLength;
            }

            if (APP_CONSOLE_CheckIsPrintable(*appConsole.pNextChar))
            {
                if (appConsole.echoEnable)
                {
                    SYS_CONSOLE_Write(SYS_CONSOLE_INDEX_0, appConsole.pNextChar, 1);
                }

                appConsole.dataLength++;
                appConsole.pNextChar++;
                appConsole.numCharToReceive--;
            }
        }
    }

    return appConsole.dataLength;
}

static bool APP_CONSOLE_CharToHex(char value, uint8_t *hex)
{
    if ((value >= '0') && (value <= '9'))
    {
        *hex = value - 0x30;
    }
    else if ((value >= 'A') && (value <= 'F'))
    {
        *hex = value - 0x37;
    }
    else if ((value >= 'a') && (value <= 'f'))
    {
        *hex = value - 0x57;
    }
    else
    {
        return 0;
    }

    return 1;
}

static bool APP_CONSOLE_SetAttenuationLevel(char *level)
{
	uint8_t attLevel;
    uint8_t attLevelHex;

    if (APP_CONSOLE_CharToHex(*level++, &attLevelHex))
    {
        attLevel = attLevelHex << 4;

        attLevelHex = *level;
        if (APP_CONSOLE_CharToHex(*level, &attLevelHex))
        {
            attLevel += attLevelHex;

            if ((attLevel <= 0x1F) || (attLevel == 0xFF)) {
                appPlcTx.plcPhyTx.attenuation = attLevel;
                return true;
            }
        }
    }

    return false;
}

static bool APP_CONSOLE_SetTransmissionPeriod(char *pTime, size_t length)
{
    uint8_t index;
    uint8_t tmpValue;
    bool result = false;

    appPlcTx.timeBetweenFrames = 0;

    for(index = length - 1; index > 0; index--)
    {
        if ((*pTime >= '0') && (*pTime <= '9')) {
				tmpValue = (*pTime - 0x30);
                appPlcTx.timeBetweenFrames += (uint32_t)pow(10, index) * tmpValue;
                pTime++;

                result = true;
        }
        else
        {
            result = false;
            break;
        }
    }

    return result;
}

static bool APP_CONSOLE_SetDataLength(char *pDataLength, size_t length)
{
    uint16_t dataLength = 0;
    uint8_t index;
    uint8_t tmpValue;
    bool result = false;

    appPlcTx.plcPhyTx.dataLength = 0;

    for (index = length; index > 0; index--)
    {
        if ((*pDataLength >= '0') && (*pDataLength <= '9')) {
				tmpValue = (*pDataLength - 0x30);
                dataLength += (uint32_t)pow(10, index - 1) * tmpValue;
                pDataLength++;
                result = true;
        }
        else
        {
            result = false;
            break;
        }
    }

    if (result & (dataLength <= APP_PLC_BUFFER_SIZE))
    {
        appPlcTx.plcPhyTx.dataLength = dataLength;
    }

    return result;
}

static bool APP_CONSOLE_SetDataMode(char *mode)
{
    size_t length;
    uint8_t* pData;
    uint8_t numBytesRnd;
    uint32_t dataValue = 0;
    bool result = true;

    length = appPlcTx.plcPhyTx.dataLength;
    pData = appPlcTx.plcPhyTx.pTransmitData;

    switch (*mode)
    {
        case '0':
            /* Random mode */
            *pData++ = (uint8_t)0x55;
            length--;
            numBytesRnd = 0;
            while(length--)
            {
                if ((numBytesRnd % 4) == 0)
                {
                    dataValue = TRNG_ReadData();
                }

                *pData++ = (uint8_t)dataValue;
                dataValue >>= 8;
                numBytesRnd++;
            }
            break;

        case '1':
            /* Fixed mode */
            dataValue = 0x30;
            while(length--)
            {
                *pData++ = (uint8_t)dataValue++;
                if (dataValue == 0x3A)
                {
                    dataValue = 0x30;
                }
            }
            break;

        default:
            result = false;
        }

    return result;
}

static bool APP_CONSOLE_SetBranchMode(char *mode)
{
    bool result = true;

    switch (*mode)
    {
        case '0':
            APP_PLC_SetImpedanceState(1, HI_STATE);
            break;

        case '1':
            APP_PLC_SetImpedanceState(0, HI_STATE);
            break;

        case '2':
            APP_PLC_SetImpedanceState(0, VLO_STATE);
            break;

        default:
            result = false;
    }

    return result;
}

static void APP_CONSOLE_ShowConfiguration(void)
{
    APP_CONSOLE_Print("\n\r-- Configuration Info --------------\r\n");
    APP_CONSOLE_Print("-I- PHY Version: 0x%08X\n\r", (unsigned int)appPlcTx.plcPhyVersion);

    if (appPlcTx.plcPhyTx.attenuation == 0xFF)
    {
        APP_CONSOLE_Print("-I- TX Attenuation: 0xFF (no signal)\n\r");
    }
    else
    {
        APP_CONSOLE_Print("-I- TX Attenuation: 0x%02X\n\r", (unsigned int)appPlcTx.plcPhyTx.attenuation);
    }

    if (appPlcTx.txAuto)
    {
        APP_CONSOLE_Print("-I- Branch Mode : Autodetect - ");
    }
    else
    {
        APP_CONSOLE_Print("-I- Branch Mode : Fixed - ");
    }

    if (appPlcTx.txImpedance == HI_STATE)
    {
        APP_CONSOLE_Print("High Impedance \r\n");
    }
    else
    {
        APP_CONSOLE_Print("Very Low Impedance \r\n");
    }

    APP_CONSOLE_Print("-I- Time Period: %u\n\r", (unsigned int)appPlcTx.timeBetweenFrames);
    APP_CONSOLE_Print("-I- Data Len: %u\n\r", (unsigned int)appPlcTx.plcPhyTx.dataLength);

    if (appPlcTx.plcPhyTx.pTransmitData[0] == 0x55)
    {
        APP_CONSOLE_Print("-I- Random Data\r\n");
    }
    else
    {
        APP_CONSOLE_Print("-I- Fixed Data\r\n");
    }
}

static void APP_CONSOLE_CalibrationRMSTaskHI(void)
{
    switch ( appConsole.calibState )
    {
        /* Calibration state machine */
        case APP_CONSOLE_CALIB_IDLE:
        {
            APP_CONSOLE_Print("\r\nGet RMS MAX High Impedance. Connect DUT against CISPR LISN (50ohm), type 's' to start or 'x' to cancel \r\n");
            appConsole.calibState = APP_CONSOLE_CALIB_RMSMAX;
            APP_CONSOLE_ReadRestart(1);
            break;
        }
        
        case APP_CONSOLE_CALIB_RMSMAX:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 's') || (appConsole.pReceivedChar[0] == 'S'))
                {
                    APP_CONSOLE_Print("\r\nGetting values...\r\n");
                    /* Launch MaxRMS process for High State impedance */
                    APP_PLC_GetCalibrationValues(APP_PLC_CALIBRATE_RMSMAX, HI_STATE);
                    
                    appConsole.calibState = APP_CONSOLE_CALIB_RMSMAX_WAITING;
                }
                else if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel PHY calibration\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                    appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
                }
                else
                {
                    if (appConsole.echoEnable) 
                    {
                        // Delete the last received character
                        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b");
                    }
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }
        
        case APP_CONSOLE_CALIB_RMSMAX_WAITING:
        {
            if (APP_PLC_CalibrationValuesAreReady((void *)&appConsole.maxRmsValuesHi))
            {
                // Show Calibration values
                uint8_t index;

                APP_CONSOLE_Print("\r\nEnd Phy calibration\r\n");
                APP_CONSOLE_Print("\r\nRMS MAX values for high impedance:");
                for (index = 0; index < SRV_PCOUP_NUM_TX_LEVELS; index++)
                {
                    APP_CONSOLE_Print(" %u,", appConsole.maxRmsValuesHi[index]);
                }
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b\r\n\r\n");

                // Go back normal state
                appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            }
            break;
        }
        
        default:
        {
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Error in Calibration process.");
            // Go back normal state
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            break;
        }
    }
}

static void APP_CONSOLE_CalibrationRMSTaskVLO(void)
{
    switch ( appConsole.calibState )
    {
        /* Calibration state machine */
        case APP_CONSOLE_CALIB_IDLE:
        {
            APP_CONSOLE_Print("\r\nGet RMS MAX Low Impedance. Connect DUT against 2ohm-LISN, type 's' to start or 'x' to cancel \r\n");
            appConsole.calibState = APP_CONSOLE_CALIB_RMSMAX;
            APP_CONSOLE_ReadRestart(1);
            break;
        }

        case APP_CONSOLE_CALIB_RMSMAX:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 's') || (appConsole.pReceivedChar[0] == 'S'))
                {
                    APP_CONSOLE_Print("\r\nGetting values...\r\n");
                    /* Launch MaxRMS process for Very low State impedance */
                    APP_PLC_GetCalibrationValues(APP_PLC_CALIBRATE_RMSMAX, VLO_STATE);

                    appConsole.calibState = APP_CONSOLE_CALIB_RMSMAX_WAITING;
                }
                else if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel PHY calibration\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                    appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
                }
                else
                {
                    if (appConsole.echoEnable)
                    {
                        // Delete the last received character
                        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b");
                    }
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_CALIB_RMSMAX_WAITING:
        {
            if (APP_PLC_CalibrationValuesAreReady((void *)&appConsole.maxRmsValuesVlo))
            {
                // Show Calibration values
                uint8_t index;

                APP_CONSOLE_Print("\r\nEnd Phy calibration\r\n");
                APP_CONSOLE_Print("\r\nRMS MAX values for low impedance:");
                for (index = 0; index < SRV_PCOUP_NUM_TX_LEVELS; index++)
                {
                    APP_CONSOLE_Print(" %u,", appConsole.maxRmsValuesVlo[index]);
                }
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b\r\n\r\n");

                // Go back normal state
                appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            }
            break;
        }

        default:
        {
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Error in Calibration process.");
            // Go back normal state
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            break;
        }
    }
}

static void APP_CONSOLE_CalibrationThresholdTask(void)
{
    switch ( appConsole.calibState )
    {
        /* Calibration state machine */
        case APP_CONSOLE_CALIB_IDLE:
        {
            APP_CONSOLE_Print("\r\nGet Thresholds for high impedance. Connect DUT against CISPR LISN (50ohm), type 's' to start or 'x' to cancel \r\n");
            appConsole.calibState = APP_CONSOLE_CALIB_THRESHOLD_HI;
            APP_CONSOLE_ReadRestart(1);
            break;
        }
        
        case APP_CONSOLE_CALIB_THRESHOLD_HI:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 's') || (appConsole.pReceivedChar[0] == 'S'))
                {
                    APP_CONSOLE_Print("\r\nGetting values...\r\n");
                    /* Launch Threshold process for High State impedance */
                    APP_PLC_GetCalibrationValues(APP_PLC_CALIBRATE_THRESHOLD, HI_STATE);
                    
                    appConsole.calibState = APP_CONSOLE_CALIB_THRESHOLD_HI_WAITING;
                }
                else if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel PHY calibration\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                    appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
                }
                else
                {
                    if (appConsole.echoEnable) 
                    {
                        // Delete the last received character
                        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b");
                    }
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }
        
        case APP_CONSOLE_CALIB_THRESHOLD_HI_WAITING:
        {
            if (APP_PLC_CalibrationValuesAreReady((void *)&appConsole.thresholdValuesHi))
            {
                appConsole.calibState = APP_CONSOLE_CALIB_THRESHOLD_LO;
            }
            break;
        }
        
        case APP_CONSOLE_CALIB_THRESHOLD_LO:
        {
            /* Launch Threshold process for High State impedance */
            APP_PLC_GetCalibrationValues(APP_PLC_CALIBRATE_THRESHOLD, VLO_STATE);

            appConsole.calibState = APP_CONSOLE_CALIB_THRESHOLD_LO_WAITING;
            break;
        }
        
        case APP_CONSOLE_CALIB_THRESHOLD_LO_WAITING:
        {
            if (APP_PLC_CalibrationValuesAreReady((void *)&appConsole.thresholdValuesVlo))
            {
                
                // Show Calibration values
                uint8_t index;
                
                APP_CONSOLE_Print("\r\nEnd Phy calibration\r\n");
                APP_CONSOLE_Print("\r\nThreshold values for high impedance:");
                for (index = 0; index < SRV_PCOUP_NUM_TX_LEVELS; index++)
                {
                    APP_CONSOLE_Print(" %u,", appConsole.thresholdValuesHi[index]);
                }
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b");
                
                APP_CONSOLE_Print("\r\nThreshold values for vlow impedance:");
                for (index = 0; index < SRV_PCOUP_NUM_TX_LEVELS; index++)
                {
                    APP_CONSOLE_Print(" %u,", appConsole.thresholdValuesVlo[index]);
                }
                SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b\r\n\r\n");
                
                // Go back normal state
                appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            }
            break;
        }
        
        default:
        {
            SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "Error in Calibration process.");
            // Go back normal state
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            appConsole.calibState = APP_CONSOLE_CALIB_IDLE;
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CONSOLE_Initialize ( void )

  Remarks:
    See prototype in app_console.h.
 */

void APP_CONSOLE_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appConsole.state = APP_CONSOLE_STATE_IDLE;

    /* Init Timer handler */
    appConsole.tmr1Handle = SYS_TIME_HANDLE_INVALID;

    /* Update state machine */
    appConsole.state = APP_CONSOLE_STATE_INIT;

    /* Init Reception data */
    appConsole.pTransmitChar = pTransmitBuffer;
    appConsole.pReceivedChar = pReceivedBuffer;
    appConsole.pNextChar = pReceivedBuffer;
    appConsole.dataLength = 0;
    appConsole.numCharToReceive = 0;

    /* Set ECHO ON by default */
    appConsole.echoEnable = true;
    
    /* Set Calibration State */
    appConsole.calibState = APP_CONSOLE_CALIB_IDLE;

}

/******************************************************************************
  Function:
    void APP_CONSOLE_Tasks ( void )

  Remarks:
    See prototype in app_console.h.
 */

void APP_CONSOLE_Tasks ( void )
{
    /* Refresh WDG */
    CLEAR_WATCHDOG();

    /* Read console port */
    APP_CONSOLE_ReadSerialChar();

    /* Check the application's current state. */
    switch ( appConsole.state )
    {
        /* Application's initial state. */
        case APP_CONSOLE_STATE_INIT:
        {
            appConsole.state = APP_CONSOLE_STATE_WAIT_PLC;

            /* Show App Header */
            APP_CONSOLE_Print(STRING_HEADER);

            break;
        }

        case APP_CONSOLE_STATE_WAIT_PLC:
        {
            /* Wait for PLC transceiver initialization */
            if (appPlc.state == APP_PLC_STATE_WAITING)
            {
                /* Show Console menu */
                appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            }
            else if (appPlc.state == APP_PLC_STATE_TX)
            {
                /* Set TX state */
                appConsole.state = APP_CONSOLE_STATE_TX;
            }
            break;
        }

        case APP_CONSOLE_STATE_SHOW_MENU:
        {
            /* Show console interface */
            APP_CONSOLE_Print(MENU_HEADER);

            /* Show console prompt */
            APP_CONSOLE_Print(MENU_PROMPT);

            /* Waiting Console command */
            appConsole.state = APP_CONSOLE_STATE_CONSOLE;
            APP_CONSOLE_ReadRestart(1);

            break;
        }

        case APP_CONSOLE_STATE_CONSOLE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                switch(*appConsole.pReceivedChar)
                {
                    case '0':
                        appConsole.state = APP_CONSOLE_STATE_SET_ATT_LEVEL;
                        APP_CONSOLE_Print("\r\nEnter attenuation level using 2 digits [00..FF][use FF for signal 0] : ");
                        APP_CONSOLE_ReadRestart(2);
                        break;

                    case '1':
                        APP_CONSOLE_Print("\r\nEnter transmission period in us. (max. 10 digits and value min 2100 us): ");
                        appConsole.state = APP_CONSOLE_STATE_SET_TIME_PERIOD;
                        APP_CONSOLE_ReadRestart(10);
                        break;

                    case '2':
                        APP_CONSOLE_Print("\r\nEnter length of data to transmit in bytes (max. 256 bytes): ");
                        appConsole.state = APP_CONSOLE_STATE_SET_DATA_LEN;
                        APP_CONSOLE_ReadRestart(3);
                        break;

                    case '3':
                        APP_CONSOLE_Print(MENU_BRANCH_MODE);
                        appConsole.state = APP_CONSOLE_STATE_SET_BRANCH_MODE;
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '4':
                        APP_CONSOLE_Print("\r\nSet PHY PLC Sniffer mode, type 'x' to cancel.\r\n");
                        APP_PLC_DataIndCallbackRegister(APP_CONSOLE_PLCDataIndicationCallback);
                        appConsole.state = APP_CONSOLE_STATE_SNIFFER_MODE;
                        appConsole.rxNumSequence = 0;
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '5':
                        APP_CONSOLE_Print("\r\nStarting PHY Calibration process...\r\n");
                        appConsole.state = APP_CONSOLE_STATE_PLC_CALIBRATION_RMSMAX_HI;
                        break;

                    case '6':
                        APP_CONSOLE_Print("\r\nStarting PHY Calibration process...\r\n");
                        appConsole.state = APP_CONSOLE_STATE_PLC_CALIBRATION_RMSMAX_VLO;
                        break;

                    case '7':
                        APP_CONSOLE_Print("\r\nStarting PHY Calibration process...\r\n");
                        appConsole.state = APP_CONSOLE_STATE_PLC_CALIBRATION_THRESHOLD;
                        break;

                    case 'v':
                    case 'V':
                        appConsole.state = APP_CONSOLE_STATE_VIEW_CONFIG;
                        break;

                    case 'e':
                    case 'E':
                        APP_CONSOLE_Print("\r\nStart transmission, type 'x' to cancel.\r\n");
                        APP_PLC_StartTramission();
                        appConsole.state = APP_CONSOLE_STATE_TX;
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case 'c':
                    case 'C':
                        if (appConsole.echoEnable)
                        {
                            appConsole.echoEnable = false;
                            APP_CONSOLE_Print("\r\nConsole ECHO disabled.\r\n");
                        }
                        else
                        {
                            appConsole.echoEnable = true;
                            APP_CONSOLE_Print("\r\nConsole ECHO enabled.\r\n");
                        }

                        appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                        break;

                    default:
                        /* Discard character */
                        appConsole.state = APP_CONSOLE_STATE_ERROR;
                        break;

                }
            }

            break;
        }

        case APP_CONSOLE_STATE_SET_ATT_LEVEL:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetAttenuationLevel(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet Attenuation level = 0x%02x\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.attenuation);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Attenuation level not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(2);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_TIME_PERIOD:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetTransmissionPeriod(appConsole.pReceivedChar, appConsole.dataLength))
                {
                    APP_CONSOLE_Print("\r\nSet Time Period = %u us.\r\n",
                            (unsigned int)appPlcTx.timeBetweenFrames);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Time Period not defined. Try again : ");
                    APP_CONSOLE_ReadRestart(10);
                }
                break;
            }
        }

        case APP_CONSOLE_STATE_SET_DATA_LEN:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetDataLength(appConsole.pReceivedChar, appConsole.dataLength))
                {
                    APP_CONSOLE_Print("\r\nSet Data Length = %u bytes\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.dataLength);

                    /* Set Data content */
                    APP_CONSOLE_Print(MENU_DATA_MODE);
                    appConsole.state = APP_CONSOLE_STATE_SET_DATA;
                    APP_CONSOLE_ReadRestart(1);
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Data length is not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(3);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_DATA:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetDataMode(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet Data mode successfully\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Data Mode not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_BRANCH_MODE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetBranchMode(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet Auto: %u, Branch: %u \r\n",
                            (unsigned int)appPlcTx.txAuto,
                            (unsigned int)appPlcTx.txImpedance);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Branch Mode not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SNIFFER_MODE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel PLC PHY Sniffer mode\r\n");
                    APP_PLC_DataIndCallbackRegister(NULL);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    if (appConsole.echoEnable) 
                    {
                        // Delete the last received character
                        SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, "\b \b");
                    }
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_PLC_CALIBRATION_RMSMAX_HI:
        {
            APP_CONSOLE_CalibrationRMSTaskHI();
            break;
        }

        case APP_CONSOLE_STATE_PLC_CALIBRATION_RMSMAX_VLO:
        {
            APP_CONSOLE_CalibrationRMSTaskVLO();
            break;
        }

        case APP_CONSOLE_STATE_PLC_CALIBRATION_THRESHOLD:
        {
            APP_CONSOLE_CalibrationThresholdTask();
            break;
        }

        case APP_CONSOLE_STATE_VIEW_CONFIG:
        {
            APP_CONSOLE_ShowConfiguration();
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            break;
        }

        case APP_CONSOLE_STATE_TX:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel transmission\r\n");
                    appPlc.state = APP_PLC_STATE_STOP_TX;
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_ERROR:
        {
            APP_CONSOLE_Print("\r\nERROR: Unknown received character\r\n");
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            break;
        }
    }
}

void APP_CONSOLE_Print(const char *format, ...)
{
    size_t len = 0;
    uint32_t numRetries;

    if (appConsole.state == APP_CONSOLE_STATE_INIT)
    {
        return;
    }

    numRetries = 1000 * SYS_CONSOLE_WriteCountGet(SYS_CONSOLE_INDEX_0);

    while(SYS_CONSOLE_WriteCountGet(SYS_CONSOLE_INDEX_0))
    {
        if (numRetries--)
        {
            /* Maintain Console service */
            SYS_CONSOLE_Tasks(SYS_CONSOLE_INDEX_0);

            /* Refresh WDG */
            CLEAR_WATCHDOG();
        }
        else
        {
            return;
        }
    }

    va_start( sArgs, format );
    len = vsnprintf(appConsole.pTransmitChar, SERIAL_BUFFER_SIZE - 1, format, sArgs);
    va_end( sArgs );

    if (len > SERIAL_BUFFER_SIZE - 1)
    {
        len = SERIAL_BUFFER_SIZE - 1;
    }

    appConsole.pTransmitChar[len] = '\0';
    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, (const char *) appConsole.pTransmitChar);
}


/*******************************************************************************
 End of File
 */
