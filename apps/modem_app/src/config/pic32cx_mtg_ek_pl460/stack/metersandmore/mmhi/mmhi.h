/*******************************************************************************
  Interface definition of Meters And More HI (Host Interface) module.

  Company:
    Microchip Technology Inc.

  File Name:
    mmhi.h

  Summary:
    Interface definition of Meters And More HI (Host Interface) module.

  Description:
    This file defines the interface for the Meters And More HI (Host Interface)
    module.
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

#ifndef MMHI_H
#define MMHI_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "system/system.h"
#include "device.h"
#include "configuration.h"
#include "stack/metersandmore/mmhi/mmhi_definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Meters And More HI Result

  Summary:
    Result of a Meters And More host interface operation.

  Description:
    Identifies the result of certain MMHI operations.
*/

typedef enum
{
    // Operation completed with success
    MMHI_SUCCESS = 0,

    // CRC error
    MMHI_ERROR_CRC,

    // Wrong argument/parameter length
    MMHI_ERROR_WPL,

    // Wrong argument/parameter value
    MMHI_ERROR_WPV,

    // Function is busy or not configured
    MMHI_ERROR_BUSY,

    // Command Unknown
    MMHI_ERROR_COMMAND,

    // Function is physically not present or not connected
    MMHI_ERROR_NOT_PRESENT,

    // Function is disabled by a software option
    MMHI_ERROR_DISABLED,

    // Operation timeout
    MMHI_ERROR_TIMEOUT,

    // Generic error
    MMHI_ERROR = 0xFF,

} MMHI_RESULT;

// *****************************************************************************
/* Meters And More Reset Cause

  Summary:
    Define the reset cause values

  Description:
    Identifies the reset cause
*/

typedef enum
{
    // Power-on or HW reset
    MMHI_RST_HW = 0,

    // Watchdog reset
    MMHI_RST_WDT,

    // SW reset
    MMHI_RST_SW,

    // BIO Request
    MMHI_RST_BIO,

    // Diagnostic: Inconsistent PLC PHY ISR received
    MMHI_RST_DIAG_PLC_ISR,

    // Diagnostic: Timer and ZERO CROSSING
    MMHI_RST_DIAG_ZC,

    // Diagnostic: FSM unreachable state
    MMHI_RST_DIAG_FSM,

    // Diagnostic: No ISR from PLC
    MMHI_RST_DIAG_NO_ISR,

} MMHI_RESET_CAUSE;

// *****************************************************************************
/* MMHI module Status

  Summary:
    Defines the status of the MMHI module.

  Description:
    This enumeration defines the status of the MMHI module:
        - MMHI_STATUS_UNINITIALIZED: MMHI module has not been initialized.
        - MMHI_STATUS_INITIALIZED: MMHI module has initialized but not opened
        - MMHI_STATUS_READY: MMHI module is ready to be used.
        - MMHI_STATUS_ERROR: An unspecified error has occurred
  Remarks:
    None.
*/
typedef enum
{
    MMHI_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    MMHI_STATUS_INITIALIZED = SYS_STATUS_READY_EXTENDED + 1,
    MMHI_STATUS_READY = SYS_STATUS_READY,
    MMHI_STATUS_ERROR = SYS_STATUS_ERROR,
} MMHI_STATUS;

// *****************************************************************************
/* Meters And More HI module Initialization Data

  Summary:
    Defines the data required to initialize the Meters And More HI module

  Description:
    Contains fields which define the information required by HI module upon
    initialization:
    - The rate at which associated task is executed
    - The role of the Device, Master or Slave

  Remarks:
    None.
*/
typedef struct
{
    /* Initialization data for the underlying device */
    const MMHI_INIT_DATA* deviceInitData;

} MMHI_INIT;

// *****************************************************************************
/* Meters And More HI Mac Data Indication Function Pointer

  Summary:
    Pointer to a Meters And More HI module Mac Data Indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More HI
    Mac Data Indication function. A client must register a pointer using the callback register 
    function whose function signature (parameter and return value types) match the types 
    specified by this function pointer in order to receive related event callbacks
    from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    dsap - Destination LSAP
    reqId - Unique Request ID for each request 
    lsdu - Pointer to the LLC data
    lsduLen - Length of the LLC data

  Returns:
    None.

  Example:
    <code>
    void APP_MyCommandFrameHandler( uint8_t dsap, uint8_t reqId, 
        uint8_t* lsdu, uint8_t lsduLen )
    {
        if (lsduLen > 0) 
        {

        }
    }
    </code>

  Remarks:
    None.
*/
typedef void ( *MMHI_MAC_DATA_IND_CALLBACK )( uint8_t dsap, uint8_t reqId, 
    uint8_t* lsdu, uint8_t lsduLen );

// *****************************************************************************
/* Meters And More HI Command Frame Indication Function Pointer

  Summary:
    Pointer to a Meters And More HI module Command Frame Indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More HI
    command frame function. A client must register a pointer using the callback register 
    function whose function signature (parameter and return value types) match the types 
    specified by this function pointer in order to receive related event callbacks
    from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    data - Pointer to the object containing any data necessary to process the MMHI
            command frame.
    length - Length of the command frame data

  Returns:
    None.

  Example:
    <code>
    void APP_MyCommandFrameHandler( uint8_t* data, uint8_t length )
    {
        if (length > 0) 
        {

        }
    }
    </code>

  Remarks:
    This Callback is only generated if a custom command has been previously 
    registered by MMHI_CommandCallbackRegister function.
*/
typedef void ( *MMHI_CMD_FRAME_IND_CALLBACK )( uint8_t* data, uint8_t length );

// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ MMHI_Initialize (
      const SYS_MODULE_INDEX index,
      const SYS_MODULE_INIT * const init
    );

  Summary:
    Initializes the Meters And More HI module according to initialization
    parameters.

  Description:
    This routine initializes the Meters And More HI module.
    The initialization data is specified by the init parameter.
    It is a single instance module and thus only one index is allowed.

  Precondition:
    The low-level processor and board initialization must be completed before
    the system can call the initialization functions for any modules.

  Parameters:
    index - Identifier for the instance to be initialized
            (single instance allowed)
    init  - Pointer to the init data structure containing any data
            necessary to initialize the module.

  Returns:
    If successful, returns a valid handle to a module instance object.
    Otherwise, it returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    SYS_MODULE_OBJ   sysObjMetersandmore;

    static const MMHI_PLIB_INTERFACE mmhiUARTPlibAPI =
{
    .readCallbackRegister = (MMHI_PLIB_READCALLBACKREGISTER)FLEXCOM7_USART_ReadCallbackRegister,
    .write = (MMHI_PLIB_WRITE)FLEXCOM7_USART_Write,
    .writeCountGet = (MMHI_PLIB_WRITE_COUNT_GET)FLEXCOM7_USART_WriteCountGet,
    .writeIsBusy = (MMHI_PLIB_WRITE_IS_BUSY)FLEXCOM7_USART_WriteIsBusy,
};

    static const MMHI_INIT_DATA mmhiUARTInitData =
{
    .uartPLIB = &mmhiUARTPlibAPI,
    .treqSignalPin = SYS_PORT_PIN_PB26
};

    MMHI_INIT mmhiInit = {
        .deviceInitData = (const void*)&mmhiUARTInitData,
        .taskRateMs = MM_STACK_TASK_RATE_MS
    };

    sysObjMetersandmore = MMHI_Initialize(0, (SYS_MODULE_INIT *)&mmhiInit);
    </code>

  Remarks:
    This routine must be called before any other HI routine is called.
*/
SYS_MODULE_OBJ MMHI_Initialize(const SYS_MODULE_INDEX index,
                              const SYS_MODULE_INIT * const init);

// *****************************************************************************
/* Function:
    DRV_HANDLE MMHI_Open (SYS_MODULE_OBJ object)

  Summary:
    Opens the specified MM host interface and returns a handle to it.

  Description:
    This routine opens the MMHI and provides a handle that must be provided 
    to all other MMHI operations to identify the caller.

    This MMHI is a single client interface, so MMHI_Open API should be
    called only once until MMHI is closed.

  Precondition:
    Function MMHI_Initialize must have been called before calling this
    function.

  Parameters:
    object - Object handle for the specified driver instance (returned from
             MMHI_Initialize)

  Returns:
    If successful, the routine returns a valid open-interface handle.

    If an error occurs, the return value is MMHI_HANDLE_INVALID. Error can occur
    - if the MMHI has been already opened once and in use.
    - if the UART peripheral instance being opened is not initialized or is
      invalid.

  Example:
    <code>
    MMHI_HANDLE handle;
    SYS_MODULE_OBJ   sysObjMetersandmore;

    handle = MMHI_Open(sysObjMetersandmore);
    if (handle == MMHI_HANDLE_INVALID)
    {

    }
    </code>

  Remarks:
    The handle returned is valid until the MMHI_Close routine is called.
    This routine will NEVER block waiting for hardware.
*/
MMHI_HANDLE MMHI_Open(SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
    MMHI_STATUS MMHI_GetStatus ( SYS_MODULE_OBJ object )

  Summary:
    Gets the status of the MMHI module.

  Description:
    This function allows to retieve MMHI module status.
    It must be used to ensure the module is Ready before start using it.

  Precondition:
    None.

  Parameters:
    object - Object handle for the specified driver instance (returned from
             MMHI_Initialize)

  Returns:
    Returns the status of the Meters And Mores MMHI module.
    - MMHI_STATUS_UNINITIALIZED: MMHI module has not been initialized.
    - MMHI_STATUS_INITIALIZED: MMHI module has initialized but not opened
    - MMHI_STATUS_READY: MMHI module is ready to be used.
    - MMHI_STATUS_ERROR: An unspecified error has occurred

  Example:
    <code>
    SYS_MODULE_OBJ   sysObjMetersandmore;

    case APP_MMHI_STATE_START:
    {
        if (MMHI_GetStatus(sysObjMetersandmore) == MMHI_STATUS_READY)
        {
            app_mmhiData.state = APP_MMHI_STATE_RUNNING;
        }

        break;
    }
    </code>

  Remarks:
    None.
*/
MMHI_STATUS MMHI_GetStatus ( SYS_MODULE_OBJ object );

/***************************************************************************
  Function:
    void MMHI_Tasks ( SYS_MODULE_OBJ object )

  Summary:
    Maintains the MM Host interface state machine.

  Description:
    This function is used to maintain the MM Host interface's internal state
    machine.

  Precondition:
    The MMHI_Initialize routine must have been called to obtain a
    valid system object handle.

  Parameters:
    object - Object handle for the specified driver instance (returned from
             MMHI_Initialize)
  Returns:
    None.

  Example:
    <code>
    SYS_MODULE_OBJ   sysObjMetersandmore;

    while (true)
    {
        MMHI_Tasks (sysObjMetersandmore);
    }
    </code>

  Remarks:
    - This function is normally not called directly by an application. It is
      called by the system's Tasks routine (SYS_Tasks).
    - This function will never block or access any resources that may cause
      it to block.
*/
void MMHI_Tasks ( SYS_MODULE_OBJ object );

// *****************************************************************************
/* Function:
    void MMHI_MacDataCallbackRegister (
        MMHI_MAC_DATA_IND_CALLBACK callback
    );

  Summary:
    Allows a client to register a callback function to be called when a mac data 
    command is received.

  Description:
    This function allows a client to register a handling function for the MMHI to 
    call back when the MAC_DATA_REQ command code has been detected.

  Parameters:
    callback - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    void APP_MyMacDataCallback(uint8_t dsap, uint8_t reqId, 
        uint8_t* lsdu, uint8_t lsduLen)
    {
        if (length > 0)
        {

        }
    }

    MMHI_MacDataCallbackRegister(APP_MyMacDataCallback);
    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving MAC DATA notifications.
*/
void MMHI_MacDataCallbackRegister(MMHI_MAC_DATA_IND_CALLBACK callback);

// *****************************************************************************
/* Function:
    MMHI_RESULT MMHI_CommandCallbackRegister (
        uint8_t cmdCode,
        MMHI_CMD_FRAME_IND_CALLBACK callback
    );

  Summary:
    Allows a client to register a new command code in MM host interface.

  Description:
    This function allows a client to register a handling function for the MMHI to call back
    when the command code has been detected.

  Parameters:
    cmdCode - New Command code to be registered in MMHI protocol
    callback - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    #define APP_MMHI_CMD_SET_STBY   0xA0

    void APP_CmdSetStbyCallback(uint8_t* data, uint8_t length)
    {
        if (length > 0)
        {

        }
    }

    MMHI_CommandCallbackRegister(APP_MMHI_CMD_SET_STBY, APP_CmdSetStbyCallback);
    </code>

  Remarks:
    Command code must be different than the internal reserved command values used in MMHI.
    Callback can be set to a NULL pointer to stop receiving notifications.
*/
MMHI_RESULT MMHI_CommandCallbackRegister(uint8_t cmdCode, MMHI_CMD_FRAME_IND_CALLBACK callback);

// *****************************************************************************
/* Function:
    void MMHI_SendCommandFrame (
        uint8_t cmdCode, 
        uint8_t* data, 
        uint8_t length
    );

  Summary:
    MMHI Send a command frame through serial port

  Description:
    Function that allow to implement a command frame for custom commands that are out of
    MMHI specification

  Precondition:
    The low-level board initialization must have been completed and
    the module's initialization function must have been called before
    the system can call the tasks routine for any module.

  Parameters:
    cmdCode - New Command code to be registered in MMHI protocol
    data - Pointer to the object containing the command frame data
    length - Length of the command frame data

  Returns:
    None.

  Example:
    <code>
    #define APP_MMHI_CMD_CUSTOM1  0x70;

    uint8_t cmdCode = APP_MMHI_CMD_CUSTOM1;
    uint8_t cmdData[5];

    cmdData[0] = DAT0;
    cmdData[1] = DAT1;
    cmdData[2] = DAT2;
    cmdData[3] = DAT3;
    cmdData[4] = DAT4;
    
    MMHI_SendCommandFrame(cmdCode, &cmdData, sizeof(cmdData));
    </code>

  Remarks:
    None.
*/
MMHI_RESULT MMHI_SendCommandFrame(uint8_t cmdCode, uint8_t* data, uint8_t length);

#ifdef __cplusplus
 }
#endif

#endif // #ifndef MMHI_H
/*******************************************************************************
 End of File
*/
