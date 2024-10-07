/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal.h

  Summary:
    Platform Abstraction Layer (PAL) Interface header file.

  Description:
    Platform Abstraction Layer (PAL) Interface header file. The PAL
    module provides a simple interface to manage the Meters And More PHY layer.
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

#ifndef PAL_H
#define PAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "system/system.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PAL Result

  Summary:
    Result of a PAL service user interface operation.

  Description:
    Lists the possible results of PAL service operations.

  Remarks:
    None.
*/
typedef enum
{
    PAL_RESULT_SUCCESS,
    PAL_RESULT_ERROR,
    PAL_RESULT_TIMEOUT,
    PAL_RESULT_BUSY_CH,
    PAL_RESULT_DENIED,
    PAL_RESULT_INVALID_PARAMETER

} PAL_RESULT;

// *****************************************************************************
/* PAL Module Status

  Summary:
    Defines the current status of the PAL module.

  Description:
    This enumeration defines the current status of the PAL module.

  Remarks:
    This enumeration is the return type for the PAL_Status routine. The
    upper layer must ensure that PAL_Status returns PAL_STATUS_READY
    before performing PAL operations.
*/
typedef enum
{
    PAL_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    PAL_STATUS_BUSY = SYS_STATUS_BUSY,
    PAL_STATUS_READY = SYS_STATUS_READY,
    PAL_STATUS_ERROR = SYS_STATUS_ERROR

} PAL_STATUS;

// *****************************************************************************
/* PAL Rx Parameters

  Summary:
    Holds PAL reception parameters.

  Description:
    This data type defines the fields containing Rx parameters for last frame.

  Remarks:
    None.
*/
typedef struct
{
    /* Frame duration referred to 1us PHY counter (Preamble + FCH + Payload) */
    uint32_t frameDuration;
    /* SNR of the header in quarters of dB (sQ13.2) */
    int16_t snrHeader;
    /* SNR of the payload in quarters of dB (sQ13.2) */
    int16_t snrPayload;
    /* NB frame (ZC info). Difference between last ZC time and payload initial time, scaled to 0-254. Value = 255: ZC not available */
    uint8_t nbRx;
    /* Link Quality */
    uint8_t lqi;
    /* Reception RSSI in dBuV */
    uint8_t rssi;

} PAL_RX_PARAMS;

// *****************************************************************************
/* PAL Data Indication Event Handler Function Pointer

   Summary
    Pointer to a PAL Data Indication event handler function.

   Description
    This data type defines the required function signature for the PAL
    reception data indication event handling callback function. When
    PAL_Init is called, a user must register a pointer whose
    function signature (parameter and return value types) matches the types
    specified by this function pointer in order to receive transfer related
    event calls back from the PAL.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    pData -     Pointer to the data buffer containing received data.

    length -    Length of the received data in bytes.

  Returns:
    None.

  Example:
    <code>
    static void _plcDataIndication(uint8_t *pData, uint16_t length)
    {

    }

    PAL_INIT palInitData;

    palInitData.palHandlers.palDataIndication = _plcDataIndication;
    palInitData.palHandlers.palTxConfirm = _plcTxConfirm;
    palInitData.palHandlers.palRxParamsIndication = _plcRxParamsIndication;

    PAL_Init(&palInitData);
    </code>

  Remarks:
    None.
*/
typedef void (*PAL_DataIndication)(uint8_t *pData, uint16_t length);

// *****************************************************************************
/* PAL Transmission Confirm Event Handler Function Pointer

   Summary
    Pointer to a PAL Transmission Confirm event handler function.

   Description
    This data type defines the required function signature for the PAL
    transmission confirm event handling callback function. When
    PAL_Init is called, a user must register a pointer whose
    function signature (parameter and return value types) matches the types
    specified by this function pointer in order to receive transfer related
    event calls back from the PAL.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    result -     PAL result corresponding to the transmission result.

  Returns:
    None.

  Example:
    <code>
    static void _plcTxConfirm(PAL_RESULT result)
    {
        switch(result)
        {
            case PAL_RESULT_SUCCESS:
                break;
            case PAL_RESULT_ERROR:
                break;
            case PAL_RESULT_TIMEOUT:
                break;
        }
    }

    PAL_INIT palInitData;

    palInitData.palHandlers.palDataIndication = _plcDataIndication;
    palInitData.palHandlers.palTxConfirm = _plcTxConfirm;
    palInitData.palHandlers.palRxParamsIndication = _plcRxParamsIndication;

    PAL_Init(&palInitData);
    </code>

  Remarks:
    If the result is PAL_RESULT_SUCCESS, it means that the data was
    transferred successfully. Otherwise, result indicates the error type.
*/
typedef void (*PAL_TxConfirm)(PAL_RESULT result);

// *****************************************************************************
/* PAL Event Handler Function Pointer to get parameters from the
   last received message

   Summary
    Pointer to a PAL event handler function to get parameters from the last
    received message.

   Description
    This data type defines the required function signature for the PAL
    reception parameters event handling callback function. When
    PAL_Init is called, a user must register a pointer whose
    function signature (parameter and return value types) matches the types
    specified by this function pointer in order to receive transfer related
    event calls back from the PAL.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    pParameters - Pointer to the parameters of the last received message.

  Returns:
    None.

  Example:
    <code>
    static PAL_RX_PARAMS rxParameters;

    static void _plcRxParamsIndication(PAL_RX_PARAMS *pParameters)
    {
        memcpy(rxParameters, pParameters, sizeof(PAL_RX_PARAMS));
    }

    PAL_INIT palInitData;

    palInitData.palHandlers.palDataIndication = _plcDataIndication;
    palInitData.palHandlers.palTxConfirm = _plcTxConfirm;
    palInitData.palHandlers.palRxParamsIndication = _plcRxParamsIndication;

    PAL_Init(&palInitData);
    </code>

  Remarks:
    This handler function is called before the data indication callback.
*/
typedef void (*PAL_RxParamsIndication)(PAL_RX_PARAMS *pParameters);

// *****************************************************************************
/* PAL Handlers Data

  Summary:
    Defines the handlers required to manage the PAL module.

  Description:
    This data type defines the handlers required to manage the PAL module.

  Remarks:
    None.
*/
typedef struct
{
    PAL_DataIndication           palDataIndication;
    PAL_TxConfirm                palTxConfirm;
    PAL_RxParamsIndication       palRxParamsIndication;
} PAL_HANDLERS;

// *****************************************************************************
/* PLC PAL Initialization Data

  Summary:
    Defines the data required to initialize the PLC PAL module.

  Description:
    This data type defines the data required to initialize the PLC PAL module.

  Remarks:
    None.
*/
typedef struct
{
    /* PAL Handlers */
    PAL_HANDLERS                 palHandlers;
} PAL_INIT;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

/* Function:
    void PAL_Init (
        PAL_INIT *init
    )

  Summary:
    Initializes the PAL module.

  Description:
    This routine initializes the PAL module, making it ready for users to
    use it. The initialization data is specified by the init parameter.

  Precondition:
    None.

  Parameters:
    init  - Pointer to the initialization data structure containing the data
            necessary to initialize the module.

  Returns:
    None.

  Example:
    <code>
    PAL_INIT palInitData;

    palInitData.palHandlers.palDataIndication = _plcDataIndication;
    palInitData.palHandlers.palTxConfirm = _plcTxConfirm;
    palInitData.palHandlers.palRxParamsIndication = _plcRxParamsIndication;

    PAL_Init(&palInitData);
    </code>

  Remarks:
    None.
*/
void PAL_Init(PAL_INIT *init);

// *****************************************************************************
/* Function:
    PAL_STATUS PAL_Status (void)

  Summary:
    Returns status of the specific instance of the PAL module.

  Description:
    This function returns the status of the specific PAL module instance.

  Precondition:
    The PAL_Init function should have been called before calling
    this function.

  Parameters:
    None.

  Returns:
    PAL_STATUS_READY - Indicates that the module is initialized and is
    ready to accept new requests from the user.

    PAL_STATUS_BUSY - Indicates that the module is busy with a previous
    initialization request from the user.

    PAL_STATUS_ERROR - Indicates that the module is in an error state.
    Any value lower than SYS_STATUS_ERROR is also an error state.

    PAL_STATUS_UNINITIALIZED - Indicates that the module is not initialized.

  Example:
    <code>
    PAL_STATUS palStatus;

    palStatus = PAL_Status();
    if (palStatus == PAL_STATUS_READY)
    {

    }
    </code>

  Remarks:
    The upper layer must ensure that PAL_Status returns PAL_STATUS_READY
    before performing PAL operations.
*/
PAL_STATUS PAL_Status(void);

// *****************************************************************************
/* Function:
    void PAL_TxRequest(uint8_t *pData, uint16_t length, uint8_t nbFrame, uint32_t delay)

  Summary:
    Allows a user to transmit data through PLC device.

  Description:
    This routine sends a new data message through PLC using the PAL module.

  Precondition:
    None.

  Parameters:
    pData -        Pointer to the data to transmit.

    length -       Length of the data to transmit in bytes.

    nbFrame -      Frame related ZeroCross information (managed by DLL).

    delay -        Delay of transmission in microseconds.

  Returns:
    None.

  Example:
    <code>
    MAC_HEADER macHeader;
    uint16_t totalLength;
    uint16_t headerLength;
    uint16_t payloadLength = 100;
    uint8_t txData[MAC_DATA_MAX_SIZE];
    uint8_t payloadData[MAC_MAX_PAYLOAD_SIZE];

    headerLength = _buildMacHeader(txData, &macHeader);

    memcpy(&txData[headerLength], payloadData, payloadLength);
    totalLength = headerLength + payloadLength;

    PAL_TxRequest(txData, totalLength, 0, 0);
    </code>

  Remarks:
    None.
*/
void PAL_TxRequest(uint8_t *pData, uint16_t length, uint8_t nbFrame, uint32_t delay);

// *****************************************************************************
/* Function:
    void PAL_Reset(void)

  Summary:
    Allows a user to reset the PAL module.

  Description:
    This routine performs a reset of the PAL module and the
    underlying PLC device.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    if (PAL_Status == PAL_STATUS_ERROR)
    {
        PAL_Reset();
    }
    </code>

  Remarks:
    None.
*/
void PAL_Reset(void);

// *****************************************************************************
/* Function:
    PAL_RESULT PAL_GetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj)

  Summary:
    Gets value of Meters And More PHY PIB attribute.

  Description:
    This routine allows a user to get information from PLC transceiver via
    PHY information base (PIB).

  Precondition:
    None.

  Parameters:
    pibObj  - Pointer to PIB object to indicate the PIB to read. PIB object
              includes a data buffer to store the read value.

  Returns:
    Result of getting the PIB (see PAL_RESULT).

  Example:
    <code>
    DRV_PLC_PHY_PIB_OBJ pibObj;
    uint32_t phyVersion;

    pibObj.id = PLC_ID_VERSION_NUM;
    pibObj.length = 4;

    if (PAL_GetPhyPib(&pibObj) == PAL_RESULT_SUCCESS)
    {
        phyVersion = *(uint32_t *)pibObj.pData;
    }
    </code>

  Remarks:
    None.
*/
PAL_RESULT PAL_GetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj);

// *****************************************************************************
/* Function:
    PAL_RESULT PAL_SetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj)

  Summary:
    Sets value of Meters And More PHY PIB attribute.

  Description:
    This routine allows a user to set information to PLC transceiver on
    PHY information base (PIB).

  Precondition:
    None.

  Parameters:
    pibObj  - Pointer to PIB object to indicate the PIB to write. PIB object
              includes a data buffer to write the new value.

  Returns:
    Result of setting the PIB (see PAL_RESULT).

  Example:
    <code>
    DRV_PLC_PHY_PIB_OBJ pibObj;

    pibObj.id = PLC_ID_CURRENT_GAIN;
    pibObj.length = 1;
    pibObj.pData[0] = 0x00;

    if (PAL_SetPhyPib(&pibObj) == PAL_RESULT_SUCCESS)
    {

    }
    </code>

  Remarks:
    None.
*/
PAL_RESULT PAL_SetPhyPib(DRV_PLC_PHY_PIB_OBJ *pibObj);

// *****************************************************************************
/* Function:
    void PAL_Tasks(void)

  Summary:
    Maintains the PAL module state machine.

  Description:
    This routine maintains the PAL module state machine.
    It has to be called periodically from upper layer so PAL layer performs its
    tasks depending on the current state.

  Precondition:
    The PAL_Init function should have been called before calling
    this function.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    PAL_INIT palInitData;

    palInitData.palHandlers.palDataIndication = _plcDataIndication;
    palInitData.palHandlers.palTxConfirm = _plcTxConfirm;
    palInitData.palHandlers.palRxParamsIndication = _plcRxParamsIndication;

    PAL_Init(&palInitData);

    while (1)
    {
        PAL_Tasks();
    }
    </code>

  Remarks:
    None.
*/
void PAL_Tasks(void);

// *****************************************************************************
/* Function:
    void PAL_MMHI_TxRequest(uint8_t *pData, uint16_t length)

  Summary:
    Allows the MM Host Interface to transmit PHY data through PLC device.

  Description:
    This routine sends a new data message through PLC using the PAL module.

  Precondition:
    None.

  Parameters:
    pData -        Pointer to the data to transmit.

    length -       Length of the data to transmit in bytes.

  Returns:
    None.

  Remarks:
    This function is only used internally from MMHI module. Don't be used by an 
    external application.
*/
void PAL_MMHI_TxRequest(uint8_t *pData, uint16_t length);

#endif // #ifndef PAL_H
/*******************************************************************************
 End of File
*/