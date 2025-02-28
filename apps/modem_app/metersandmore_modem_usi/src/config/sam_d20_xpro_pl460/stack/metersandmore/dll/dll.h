/*******************************************************************************
  Interface definition of Meters And More DLL (Data Link Layer) module.

  Company:
    Microchip Technology Inc.

  File Name:
    dll.h

  Summary:
    Interface definition of Meters And More DLL (Data Link Layer) module.

  Description:
    This file defines the interface for the Meters And More DLL (Data Link Layer)
    module.
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

#ifndef DLL_H
#define DLL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driver/driver.h"
#include "system/system.h"
#include "device.h"
#include "configuration.h"

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

/* Sizes */

/* Max LSDU Data length according to Meters And More */
#define MAX_LENGTH_432_DATA         (128U)

/* DLL object sizes */
#define MAX_ROUTE_SIZE              (9U)
#define MAC_ADDRESS_SIZE            (6U)

/* Maximum length of an IB object */
#define DLL_IB_MAX_VALUE_LENGTH     (MAC_ADDRESS_SIZE)

/* Maximum length of a MAC Event Value */
#define MAC_EVENT_VALUE_MAX_LENGTH  (MAC_ADDRESS_SIZE)

// *****************************************************************************
/* Meters And More DLL module Result

  Summary:
    Result of a Meters And More DLL module interface operation.

  Description:
    Lists the possible results of Meters And More DLL module operations.
*/
typedef enum
{
    DLL_SUCCESS,
    DLL_ERROR
} DLL_RESULT;

// *****************************************************************************
/* DSAP definition

   Summary:
    DSAP (Destination Service Access Point) values for Meters And More DLL layer.

   Description:
    This enumeration identifies the possible DSAP values
    0 - Application Frame
    2 - Network Management

   Remarks:
    None.
*/
typedef enum
{
    DLL_DSAP_APPLICATION_FRAME  = 0x00,
    DLL_DSAP_NETWORK_MANAGEMENT = 0x02
} DLL_DSAP;

// *****************************************************************************
/* ECC definition

   Summary:
    ECC (Encryption Coding Control) values for Meters And More DLL layer.

   Description:
    This enumeration identifies the possible ECC values (SSAP in older spec
    versions).
    The following values are defined:
    0 - Disabled
    1 - AES-CTR Encryption with READ key
    2 - AES-ECB Encryption with READ key
    5 - AES-CTR Encryption with WRITE key
    6 - AES-ECB Encryption with WRITE key
    (Other values are reserved).

   Remarks:
    None.
*/
typedef enum
{
  DLL_ECC_DISABLED = 0x00,
  DLL_ECC_AES_CTR_READ_KEY = 0x01,
  DLL_ECC_AES_ECB_READ_KEY = 0x02,
  DLL_ECC_AES_CTR_WRITE_KEY = 0x05,
  DLL_ECC_AES_ECB_WRITE_KEY = 0x06,
} DLL_ECC;

// *****************************************************************************
/* Service Class definition

   Summary:
    Identifies the possible MAC Service Class values as defined in
    Meters And More standard.

   Description:
    This enumeration identifies the possible MAC Service Class values
    S - Send/NoReply
    RA - Request/Respond on A subnetwork
    RB - Request/Respond involving A and B subnetworks
    RC - Request/MultiRespond

   Remarks:
    None.
*/
typedef enum
{
    SERVICE_CLASS_S  = 0x00,
    SERVICE_CLASS_RA = 0x01,
    SERVICE_CLASS_RB = 0x02,
    SERVICE_CLASS_RC = 0x03
} SERVICE_CLASS;

// *****************************************************************************
/* Tx Status definition

   Summary:
    Transmission results defined in DLL layer.

   Description:
    This enumeration identifies the possible Transmission result values
    0 - Success
    1 - Error

   Remarks:
    None.
*/
typedef enum
{
    DLL_TX_STATUS_SUCCESS = 0x00,
    DLL_TX_STATUS_ERROR   = 0x01
} DLL_TX_STATUS;

// *****************************************************************************
/* MAC Address definition

   Summary:
    Defines the Address as an array of 6 unsigned 8-bit integers.

   Description:
    Creates an unsigned 8-bit integer array specific type for Address
    definition.

   Remarks:
    None.
*/
typedef struct
{
    uint8_t address[MAC_ADDRESS_SIZE];
} MAC_ADDRESS;

// *****************************************************************************
/* Routing Table Entry definition

   Summary:
    Defines the fields of an entry in the Routing Table.

   Description:
    This structure contains the fields which define a Routing Table entry.
    This table contains a route to every node in the Network.
    A Routing Table Entry is pased to DLL in DLL_DataRequest, containing
    the ordered list of the addresses of involved repeaters up to final
    destination.

   Remarks:
    The Routing Table and the Route parameter on Data Request are only
    present and used in Master Node.
*/
typedef struct
{
    MAC_ADDRESS macAddress[MAX_ROUTE_SIZE];
    uint8_t routeSize;
} ROUTING_ENTRY;

// *****************************************************************************
/* DLL Parameter Information Base definition

   Summary:
    Lists the available objects in the DLL Information Base (IB).

   Description:
    DLL IB is a collection of objects that can be read/written in order to
    retrieve information and/or configure the DLL layer.

   Remarks:
    None.
*/
typedef enum
{
    MAC_ACA_ADDRESS_IB = 0x201,
    MAC_SCA_ADDRESS_IB = 0x202,
    MAC_BAUDRATE_IB = 0x203,
    MAC_TIME_SLOT_US_IB = 0x204,
    MAC_TIME_ELABORATION_US_IB = 0x205,
    MAC_ADDITIONAL_DELAY_US_IB = 0x206,
    MAC_LAST_RX_IN_PHASE_IB = 0x207,
    MAC_LAST_RX_NB_FRAME_IB = 0x208,
    MAC_LAST_RX_SIGNAL_LEVEL_IB = 0x209,
    MAC_LAST_RX_SNR_IB = 0x20A,
    MAC_ESTIMATED_IMPDEDANCE_IB = 0x20B,
} DLL_IB_ATTRIBUTE;

/* Masks to distinguish between layer attributes */
#define MAC_ATTRIBUTE_MASK   0x200
#define LLC_ATTRIBUTE_MASK   0x400

// *****************************************************************************
/* DLL IB Value definition

   Summary:
    Defines the fields of an IB Value object.

   Description:
    This structure contains the fields which define an IB Value object,
    which contains information of its length and the value itself coded into an
    8-bit array format.

   Remarks:
    None.
*/
typedef struct
{
    uint8_t length;
    uint8_t value[DLL_IB_MAX_VALUE_LENGTH];
} DLL_IB_VALUE;

// *****************************************************************************
/* MAC Event IDs

   Summary:
    Identifies the possible MAC Event identifiers.

   Description:
    This enumeration identifies the possible MAC Event identifiers.

   Remarks:
    None.
*/
typedef enum
{
    MAC_EVENT_ID_ACA = 0
} MAC_EVENT_ID;

// *****************************************************************************
/* MAC Event Struct

   Summary:
    Contains fields defining a MAC Event

   Description:
    Contains fields of a MAC Event, its length and value itself.

   Remarks:
    None.
*/
typedef struct
{
    uint8_t length;
    uint8_t value[MAC_EVENT_VALUE_MAX_LENGTH];
} MAC_EVENT_VALUE;

// *****************************************************************************
/* Meters And More DLL Data request struct

  Summary:
    DLL Data Request Parameters Structure.

   Description:
    Contains fields which define the DLL Data Request input parameter.

  Remarks:
    Fields marked as Master Only are used in Master Node and ignored in
    Slave Nodes.
*/
typedef struct
{
  /* Pointer to the data to be sent (max length: MAX_LENGTH_432_DATA) */
  uint8_t *lsdu;
  /* Destination LSAP */
  DLL_DSAP dsap;
  /* ECC (Encryption Coding Control) */
  DLL_ECC ecc;
  /* Service class - MASTER ONLY */
  SERVICE_CLASS serviceClass;
  /* Destination route - MASTER ONLY */
  ROUTING_ENTRY dstAddress;
  /* Max length of the response - MASTER ONLY */
  uint16_t maxResponseLen;
  /* Number of time slots alocated in data request with Service_Class RC - MASTER ONLY */
  uint16_t timeSlotNum;
  /* Length of the data */
  uint16_t lsduLen;
} DLL_DATA_REQUEST_PARAMS;


// *****************************************************************************
/* Meters And More DLL Data Indication struct

  Summary:
    DLL Data Indication Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    DLL Data Indication Callback.

  Remarks:
    None.
*/
typedef struct
{
  /* Destination LSAP */
  DLL_DSAP dsap;
  /* ECC (Encryption Coding Control) */
  DLL_ECC ecc;
  /* Source address */
  MAC_ADDRESS srcAddress;
  /* Pointer to received data (max length: MAX_LENGTH_432_DATA)*/
  uint8_t *lsdu;
  /* Length of the data */
  uint16_t lsduLen;
  /* Poll Flag from LLC */
  bool pollFlag;
} DLL_DATA_IND_PARAMS;

// *****************************************************************************
/* Meters And More DLL Data Confirm struct

  Summary:
    DLL Data Confirm Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    DLL Data Confirm Callback.

  Remarks:
    None.
*/
typedef struct
{
    /* Destination LSAP */
    DLL_DSAP dsap;
    /* ECC (Encryption Coding Control) */
    DLL_ECC ecc;
    /* Destination address */
    MAC_ADDRESS dstAddress;
    /* Tx status */
    DLL_TX_STATUS txStatus;
} DLL_DATA_CONFIRM_PARAMS;

// *****************************************************************************
/* Meters And More DLL Event Indication struct

  Summary:
    DLL Event Indication Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    DLL Event Indication Callback.

  Remarks:
    This Struct is only used in Master Node.
*/
typedef struct
{
    /* Event Identifier */
    MAC_EVENT_ID eventId;
    /* First additional information */
    MAC_EVENT_VALUE eventValue;
} DLL_EVENT_IND_PARAMS;

// *****************************************************************************
/* Meters And More DLL module Initialization Data

  Summary:
    Defines the data required to initialize the Meters And More DLL module

  Description:
    Contains fields which define the information required by DLL module upon
    initialization:
    - The rate at which associated task is executed
    - The role of the Device, Master or Slave

  Remarks:
    None.
*/
typedef struct
{
  /* DLL task rate in milliseconds */
  uint8_t taskRateMs;

  /* Is master node (false in slave node) */
  bool isMaster;
} DLL_INIT;

// *****************************************************************************
/* Meters And More DLL module Data Indication Function Pointer

  Summary:
    Pointer to a Meters And More DLL module DL_Data.indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More DLL
    module DL_Data.indication callback function. A client must
    register a pointer using the callback register function whose function
    signature (parameter and return value types) match the types specified by
    this function pointer in order to receive related event callbacks
    from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    indParams - Pointer to the object containing any data necessary to process the
             DL_Data.indication primitive.

  Returns:
    None.

  Example:
    <code>
    void APP_MyDataIndEventHandler( DLL_DATA_IND_PARAMS *indParams )
    {
        if(indParams->lsduLen > 0)
        {

        }
    }
    </code>
*/
typedef void ( *DLL_DATA_IND_CALLBACK )( DLL_DATA_IND_PARAMS *indParams );

// *****************************************************************************
/* Meters And More DLL module Data Confirm Function Pointer

  Summary:
    Pointer to a Meters And More DLL module Data Confirm Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More DLL
    module Data Confirm callback function. A client must
    register a pointer using the callback register function whose function
    signature (parameter and return value types) match the types specified by
    this function pointer in order to receive related event callbacks
    from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    cfmParams - Pointer to the object containing data related to the
             result of last transmission.

  Returns:
    None.

  Example:
    <code>
    void APP_MyDataCfmEventHandler( DLL_DATA_CONFIRM_PARAMS *cfmParams )
    {
        switch(cfmParams->txStatus)
        {
            case DLL_TX_STATUS_SUCCESS:
                break;
            case DLL_TX_STATUS_ERROR:
                break;
        }
    }
    </code>

  Remarks:
    - If the status field is DLL_TX_STATUS_SUCCESS, it means that the data was
      transferred successfully.

    - Otherwise, it means that the data was not transferred successfully.

*/
typedef void ( *DLL_DATA_CONFIRM_CALLBACK )( DLL_DATA_CONFIRM_PARAMS *cfmParams );


// *****************************************************************************
/* Meters And More DLL module Event Indication Function Pointer

  Summary:
    Pointer to a Meters And More DLL module Event Indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More DLL
    module Event Indication callback function. A client must
    register a pointer using the callback register function whose function
    signature (parameter and return value types) match the types specified by
    this function pointer in order to receive related event callbacks
    from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    indParams - Pointer to the object containing any data necessary to process the
             Event Indication primitive.

  Returns:
    None.

  Example:
    <code>
    void APP_MyEventIndEventHandler( DLL_EVENT_IND_PARAMS *indParams )
    {
        if (indParams->eventId == MAC_EVENT_ID_ACA){

        }
    }
    </code>

  Remarks:
    This Callback is only generated in Master Node. There is no need to handle
    it on Slave Nodes.
*/
typedef void ( *DLL_EVENT_IND_CALLBACK )( DLL_EVENT_IND_PARAMS *indParams );


// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ DLL_Initialize (
      const SYS_MODULE_INDEX index,
      const SYS_MODULE_INIT * const init
    );

  Summary:
    Initializes the Meters And More DLL module according to initialization
    parameters.

  Description:
    This routine initializes the Meters And More DLL module.
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

    DLL_INIT dllInitData = {
        .taskRateMs = 1U,
        .isMaster = false
    };

    sysObjMetersandmore = DLL_Initialize(0, (SYS_MODULE_INIT *)&dllInitData);
    </code>

  Remarks:
    This routine must be called before any other DLL routine is called.
*/
SYS_MODULE_OBJ DLL_Initialize(const SYS_MODULE_INDEX index,
                              const SYS_MODULE_INIT * const init);


// *****************************************************************************
/* Function:
    DLL_RESULT DLL_DataIndicationCallbackRegister (
        DLL_DATA_IND_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More DLL DL_Data.indication
    event handling function for the module to call back when the requested
    transmission has finished.

  Description:
    This function allows a client to register a Meters And More
    DLL DL_Data.indication event handling function for the module to call back
    when a Meters And More DLL data indication event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Rx_Ind_callback(DLL_DATA_IND_PARAMS *indParams)
    {
        if (indParams->lsduLen > 0){

        }
    }

    DLL_DataIndicationCallbackRegister(APP_Rx_Ind_callback);
    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving notifications.
*/
DLL_RESULT DLL_DataIndicationCallbackRegister(DLL_DATA_IND_CALLBACK callback);

// *****************************************************************************
/* Function:
    DLL_RESULT DLL_DataConfirmCallbackRegister (
        DLL_DATA_CONFIRM_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More DLL DL_Data.confirm
    event handling function for the module to call back when the requested
    transmission has finished.

  Description:
    This function allows a client to register a Meters And More
    DLL DL_Data.confirm event handling function for the module to call back
    when a Meters And More DLL data confirm event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Tx_Cfm_callback(DLL_DATA_CONFIRM_PARAMS *cfmParams)
    {
        if (cfmParams->result == DLL_SUCCESS)
        {

        }
        else
        {

        }
    }

    DLL_DataConfirmCallbackRegister(APP_Tx_Cfm_callback);

    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving notifications.
*/
DLL_RESULT DLL_DataConfirmCallbackRegister(DLL_DATA_CONFIRM_CALLBACK callback);

// *****************************************************************************
/* Function:
    DLL_RESULT DLL_EventIndicationCallbackRegister (
        DLL_EVENT_IND_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More DLL DL_Event.indication
    event handling function for the module to call back
    when DLL generates a new event.

  Description:
    This function allows a client to register a Meters And More
    DLL DL_Event.indication event handling function for the module to call back
    when a Meters And More DLL event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Event_Ind_callback(DLL_EVENT_IND_CALLBACK *indParams)
    {
        if (indParams->eventId == MAC_EVENT_ID_ACA){

        }
    }

    DLL_EventIndicationCallbackRegister(APP_Event_Ind_callback);
    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving notifications.
    This Callback is only generated in Master Node. There is no need to set
    a handling function on Slave Nodes.
*/
DLL_RESULT DLL_EventIndicationCallbackRegister(DLL_EVENT_IND_CALLBACK callback);

// *****************************************************************************
/* Function:
    void DLL_DataRequest (
        DLL_DATA_REQUEST_PARAMS *drParams
    );

  Summary:
    DLL Data request

  Description:
    Function that implements the DLL DL_Data.request primitive

  Precondition:
    The low-level board initialization must have been completed and
    the module's initialization function must have been called before
    the system can call the tasks routine for any module.

  Parameters:
    drParams - Pointer to structure containing parameters related to DL_Data.request

  Returns:
    None.

  Example:
    <code>
    DLL_DATA_REQUEST_PARAMS drParams;
    ROUTING_ENTRY addr;

    drParams.dsap = DLL_DSAP_APPLICATION_FRAME;
    drParams.ecc = DLL_ECC_DISABLED;
    drParams.lsdu = appPlcTxDataBuffer;
    drParams.serviceClass = SERVICE_CLASS_RA;
    drParams.dstAddress = addr;
    drParams.maxResponseLen = 128;
    drParams.timeSlotNum = 8;
    drParams.lsduLen = 10;

    DLL_DataRequest(&drParams);
    </code>

  Remarks:
    None.
*/
void DLL_DataRequest(DLL_DATA_REQUEST_PARAMS *drParams);

// *****************************************************************************
/* Function:
    uint32_t DLL_GetTxTimeout(void);

  Summary:
    Gets the timeout that Upper Layer has to wait after a Data Request,
    according to previous Data Request info

  Description:
    This function returns the timeout that Upper Layer has to wait after issuing
    a Data Request before allowing further transmission.
    Function is implemented inside DLL layer, as it has all the information needed
    to perform the calculations.

  Precondition:
    DLL_DataRequest has to be called before calling this function, so DLL has the
    correct parameters information before performing the calculation.

  Parameters:
    None

  Returns:
    Timeout, in microseconds.

  Example:
    <code>
    DLL_DATA_REQUEST_PARAMS drParams;
    ROUTING_ENTRY addr;
    uint32_t responseWaitTimeout;

    drParams.dsap = DLL_DSAP_APPLICATION_FRAME;
    drParams.ecc = DLL_ECC_DISABLED;
    drParams.lsdu = appPlcTxDataBuffer;
    drParams.serviceClass = SERVICE_CLASS_RA;
    drParams.dstAddress = addr;
    drParams.maxResponseLen = 128;
    drParams.timeSlotNum = 8;
    drParams.lsduLen = 10;

    DLL_DataRequest(&drParams);

    responseWaitTimeout = DLL_GetTxTimeout();
    </code>

  Remarks:
    None.
*/
uint32_t DLL_GetTxTimeout(void);

// *****************************************************************************
/* Function:
    SYS_STATUS DLL_GetStatus(void);

  Summary:
    Gets the status of the DLL module.

  Description:
    This function allows to retrieve DLL module status.
    It must be used to ensure the module is Ready before start using it.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    Returns the status of the Meters And Mores DLL module.
    - SYS_STATUS_UNINITIALIZED: DLL module has not been initialized.
    - SYS_STATUS_BUSY: DLL module is busy in the process of initialization.
    - SYS_STATUS_READY: DLL module is ready to be used.

  Example:
    <code>
    case APP_DLL_STATE_START:
    {
        if (DLL_GetStatus() == SYS_STATUS_READY)
        {
            app_DLLData.state = APP_DLL_STATE_RUNNING;
        }

        break;
    }
    </code>

  Remarks:
    None.
*/
SYS_STATUS DLL_GetStatus(void);

// *****************************************************************************
/* Function:
    void DLL_Tasks (
        SYS_MODULE_OBJ object
    );

  Summary:
    Routine that maintains the state machine in the DLL module.

  Description:
    Routine that performs the tasks necessary to maintain the state machine in
    the Meters And More DLL module.

  Precondition:
    The low-level board initialization must have been completed and
    the module's initialization function must have been called before
    the system can call the tasks routine for any module.

  Parameters:
    object - Handle to the module instance

  Returns:
    None.

  Example:
    <code>
    SYS_MODULE_OBJ   sysObjMetersandmore;

    DLL_INIT dllInitData = {
        .taskRateMs = 1U,
        .isMaster = false
    };

    sysObjMetersandmore = DLL_Initialize(0, (SYS_MODULE_INIT *)&dllInitData);

    DLL_Tasks(sysObjMetersandmore);
    </code>

  Remarks:
    None.
*/
void DLL_Tasks(SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
    DLL_RESULT DLL_GetRequest (
        DLL_IB_ATTRIBUTE attribute,
        uint16_t index,
        DLL_IB_VALUE *ibValue
    );

  Summary:
    The DLL_GetRequest primitive gets the value of an attribute in the
    DLL layer Parameter Information Base (IB).

  Description:
    GetRequest primitive is used to get the value of an IB.
    Result is provided upon function call return, in the ibValue parameter.

  Precondition:
    None.

  Parameters:
    attribute - Identifier of the Attribute to retrieve value

    index - Index of element in case Attribute is a table
            Otherwise index must be set to '0'

    ibValue - Pointer to DLL_IB_VALUE object where value will be returned

  Returns:
    Result of get operation as a DLL_RESULT Enum.

  Example:
    <code>
    DLL_RESULT result;
    DLL_IB_VALUE value;
    result = DLL_GetRequest(MAC_ADDITIONAL_DELAY_US_IB, 0, &value);
    if (result == DLL_RESULT_SUCCESS)
    {

    }
    </code>

  Remarks:
    None.
*/
DLL_RESULT DLL_GetRequest(DLL_IB_ATTRIBUTE attribute, uint16_t index,
                          DLL_IB_VALUE *ibValue);

// *****************************************************************************
/* Function:
    DLL_RESULT DLL_SetRequest (
        DLL_IB_ATTRIBUTE attribute,
        uint16_t index,
        const DLL_IB_VALUE *ibValue
    );

  Summary:
    The DLL_SetRequest primitive sets the value of an attribute in the
    DLL layer Parameter Information Base (IB).

  Description:
    SetRequest primitive is used to set the value of an IB.
    Result of set operation is provided upon function call return,
    in the return result code.

  Precondition:
    None.

  Parameters:
    attribute - Identifier of the Attribute to provide value

    index - Index of element in case Attribute is a table
            Otherwise index must be set to '0'

    ibValue - Pointer to DLL_IB_VALUE object where value is contained

  Returns:
    Result of set operation as a DLL_RESULT Enum.

  Example:
    <code>
    DLL_RESULT result;
    uint32_t time = 20000;
    const DLL_IB_VALUE value;

    value.length = 4;
    memcpy(value.value, time, sizeof(time));

    result = DLL_SetRequest(MAC_ADDITIONAL_DELAY_US_IB, 0, &value);
    if (result == DLL_RESULT_SUCCESS)
    {

    }
    </code>

  Remarks:
    None.
*/
DLL_RESULT DLL_SetRequest(DLL_IB_ATTRIBUTE attribute, uint16_t index,
                          const DLL_IB_VALUE *ibValue);

#ifdef __cplusplus
 }
#endif

#endif // #ifndef DLL_H
/*******************************************************************************
 End of File
*/
