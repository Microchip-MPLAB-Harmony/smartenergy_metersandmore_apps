/*******************************************************************************
  Interface definition of Meters And More AL (Application Layer) module.

  Company:
    Microchip Technology Inc.

  File Name:
    al.h

  Summary:
    Interface definition of Meters And More AL (Application Layer) module.

  Description:
    This file defines the interface for the Meters And More AL (Application
    Layer) module.
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

#ifndef AL_H
#define AL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "stack/metersandmore/dll/dll.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* Security (Key, LMON, DATE-TIME TMAC lengths) */
#define AL_KEY_LENGTH           16U
#define AL_LMON_LENGTH          8U
#define AL_DATETIME_LENGTH      8U
#define AL_TMAC_LENGTH          8U

/* Maximum length of AL data (APDU, DLL payload without message attribute byte) */
#define AL_MAX_DATA_LENGTH            (MAX_LENGTH_432_DATA - 1U)

/* Maximum length of AL data if message is authenticated */
#define AL_MAX_DATA_AUTH_LENGTH       (AL_MAX_DATA_LENGTH - AL_DATETIME_LENGTH - AL_TMAC_LENGTH)

/* Maximum length of an IB object */
#define AL_IB_MAX_VALUE_LENGTH        (AL_KEY_LENGTH)

/* Payload to indicate LMON mismatch */
#define AL_NACK_AUTH_PAYLOAD    10U

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* AL Event IDs

   Summary:
    Identifies the possible AL Event identifiers.

   Description:
    This enumeration identifies the possible AL Event identifiers.

   Remarks:
    None.
*/
typedef enum
{
    AL_EVENT_ID_MAC_ACA = 0,
    AL_EVENT_ID_MASTER_TX_TIMEOUT = 0x20,
} AL_EVENT_ID;

// *****************************************************************************
/* AL Event Struct

   Summary:
    Contains fields defining an AL Event

   Description:
    Contains fields of a AL Event, its length and value itself.

   Remarks:
    None.
*/
typedef struct
{
    uint8_t length;
    uint8_t value[MAC_EVENT_VALUE_MAX_LENGTH];
} AL_EVENT_VALUE;

// *****************************************************************************
/* AL and DLL Parameter Information Base definition

   Summary:
    Lists the available objects in the AL and DLL Information Base (IB).

   Description:
    AL and DLL IB is a collection of objects that can be read/written in order
    to retrieve information and/or configure the AL and DLL layers.

   Remarks:
    None.
*/
typedef enum
{
    /* LMON used for Authentication (only for Meter) */
    AL_AUTH_LMON_IB = 0x001,
    /* Authentication Write Key (K1) */
    AL_AUTH_WRITE_KEY_K1_IB = 0x002,
    /* Authentication Read Key (K2) */
    AL_AUTH_READ_KEY_K2_IB = 0x003,
    /* ACA (Absolute Communication Address) of destination node, used for Authentication (only for DCU) */
    AL_AUTH_DESTINATION_NODE_ACA_IB = 0x004,
    /* TCT used for Network Management, from 1 to 255 (only for Meter) */
    AL_NM_TCT_IB = 0x005,
    /* Maximum number of retries for DLL_DataRequest */
    AL_TX_RETRY_LIMIT = 0x006,

    AL_MAC_ACA_ADDRESS_IB = 0x201,
    AL_MAC_SCA_ADDRESS_IB = 0x202,
    AL_MAC_BAUDRATE_IB = 0x203,
    AL_MAC_TIME_SLOT_US_IB = 0x204,
    AL_MAC_TIME_ELABORATION_US_IB = 0x205,
    AL_MAC_ADDITIONAL_DELAY_US_IB = 0x206,
    AL_MAC_LAST_RX_IN_PHASE_IB = 0x207,
    AL_MAC_LAST_RX_NB_FRAME_IB = 0x208,
    AL_MAC_LAST_RX_SIGNAL_LEVEL_IB = 0x209,
    AL_MAC_LAST_RX_SNR_IB = 0x20A,
    AL_MAC_ESTIMATED_IMPDEDANCE_IB = 0x20B,
    AL_LLC_IS_DCU_IB = 0x401,
} AL_IB_ATTRIBUTE;

// *****************************************************************************
/* AL Message Attributes definition

   Summary:
    Lists the available message attributes in the Meters And More Application
    Layer.

   Description:
    This data type lists the available message attributes in the Meters And More
    Application Layer.

    Each message starts with an attribute (ATTR) that contains the command code
    of the message. The length of this field is 1 byte.

   Remarks:
    None.
*/
typedef enum
{
    /* Reading operations */
    AL_MSG_READ_REQ              = 2U,
    AL_MSG_READ_REQ_AUTH         = 102U,
    AL_MSG_READ_RESP             = 3U,
    AL_MSG_READ_RESP_AUTH        = 103U,
    AL_MSG_READTAB_REQ_SEL       = 6U,
    AL_MSG_READTAB_REQ_SEL_AUTH  = 106U,
    AL_MSG_READTAB_RESP_SEL      = 7U,
    AL_MSG_READTAB_RESP_SEL_AUTH = 107U,
    AL_MSG_READTAB_REQ           = 8U,
    AL_MSG_READTAB_REQ_AUTH      = 108U,
    AL_MSG_READTAB_RESP          = 9U,
    AL_MSG_READTAB_RESP_AUTH     = 109U,
    AL_MSG_BUFF_SELECT_REQ       = 22U,
    AL_MSG_BUFF_SELECT_REQ_AUTH  = 122U,
    AL_MSG_BUFF_SELECT_RESP      = 23U,
    AL_MSG_BUFF_SELECT_RESP_AUTH = 123U,
    AL_MSG_GETTAB_REQ            = 30U,
    AL_MSG_GETTAB_REQ_AUTH       = 130U,
    AL_MSG_GETTAB_RESP           = 31U,
    AL_MSG_GETTAB_RESP_AUTH      = 131U,

    /* Writing operations */
    AL_MSG_WRITE_REQ             = 4U,
    AL_MSG_WRITE_REQ_AUTH        = 104U,
    AL_MSG_WRITETAB_REQ          = 10U,
    AL_MSG_WRITETAB_REQ_AUTH     = 110U,
    AL_MSG_SETTAB_REQ            = 14U,
    AL_MSG_SETTAB_REQ_AUTH       = 114U,
    AL_MSG_RESETTAB_REQ          = 16U,
    AL_MSG_RESETTAB_REQ_AUTH     = 116U,
    AL_MSG_SETIC_REQ             = 40U,
    AL_MSG_SETIC_REQ_AUTH        = 140U,
    AL_MSG_WRITETABIC_REQ        = 42U,
    AL_MSG_WRITETABIC_REQ_AUTH   = 142U,

    /* Network Management */
    AL_MSG_ADDRESS_REQ           = 90U,
    AL_MSG_ADDRESS_RESP          = 91U,
    AL_MSG_TCT_SET_REQ           = 92U,
    AL_MSG_REQADDR_REQ           = 94U,
    AL_MSG_REQADDR_RESP          = 95U,
    AL_MSG_NACK_RESP             = 247U,

    /* Special message */
    AL_MSG_COMMAND               = 18U,
    AL_MSG_COMMAND_AUTH          = 118U,
    AL_MSG_DATASPONT             = 20U,
    AL_MSG_DATASPONT_AUTH        = 120U,
    
    /* Software download */
    AL_MSG_REPROG_LOCAL          = 100U,
    AL_MSG_REPROG_BROADCAST      = 101U,

    /* LMON synchronization */
    AL_MSG_CHALLENGE_REQ         = 112U,
    AL_MSG_CHALLENGE_RESP        = 113U,
    
    /* Acknowledgements */
    AL_MSG_ACK_A_NODE            = 253U,
    AL_MSG_ACK_A_NODE_AUTH       = 243U,
    AL_MSG_NACK_A_NODE           = 255U,
    AL_MSG_NACK_A_NODE_AUTH      = 245U,
    AL_MSG_ACK_B_NODE            = 251U,
    AL_MSG_ACK_B_NODE_AUTH       = 241U,
    AL_MSG_NACK_B_NODE           = 249U,
    AL_MSG_NACK_B_NODE_AUTH      = 239U,

} AL_MSG_ATTR;

// *****************************************************************************
/* AL Tx Status definition

   Summary:
    Transmission results defined in AL layer.

   Description:
    This enumeration identifies the possible Transmission result values
    0 - Success
    1 - Error

   Remarks:
    None.
*/
typedef enum
{
    AL_TX_STATUS_SUCCESS = 0x00,
    AL_TX_STATUS_ERROR_BUSY = 0x01,
    AL_TX_STATUS_ERROR_BAD_LEN = 0x02,
    AL_TX_STATUS_ERROR_AES_CMAC = 0x03,
    AL_TX_STATUS_ERROR_AES_ENCRYPT = 0x04,
    AL_TX_STATUS_ERROR_AES_NO_KEY = 0x05,
    AL_TX_STATUS_ERROR_RETRY_LIMIT = 0x06
} AL_TX_STATUS;

// *****************************************************************************
/* AL Rx Status definition

   Summary:
    Reception results defined in AL layer.

   Description:
    This enumeration identifies the possible Reception result values
    0 - Success
    1 - Error

   Remarks:
    None.
*/
typedef enum
{
    AL_RX_STATUS_SUCCESS = 0x00,
    AL_RX_STATUS_ERROR_BAD_LEN = 0x01,
    AL_RX_STATUS_ERROR_AES_CMAC = 0x02,
    AL_RX_STATUS_ERROR_AES_DECRYPT = 0x03,
    AL_RX_STATUS_ERROR_AES_NO_KEY = 0x04,
    AL_RX_STATUS_ERROR_BAD_TMAC = 0x05,
} AL_RX_STATUS;

// *****************************************************************************
/* AL and DLL IB Value definition

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
    uint8_t value[AL_IB_MAX_VALUE_LENGTH];
} AL_IB_VALUE;

// *****************************************************************************
/* Meters And More AL Data Indication struct

  Summary:
    AL Data Indication Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    AL Data Indication Callback.

  Remarks:
    None.
*/
typedef struct
{
  /* DATE-TIME (only in authenticated messages) */
  uint64_t datetime;
  /* LMON of Meter used for authentication, updated with this indication */
  uint64_t lmon;
  /* Pointer to AL received data (without message attribute)*/
  uint8_t *apdu;
  /* Length of the AL data */
  uint16_t apduLen;
  /* Source address */
  MAC_ADDRESS srcAddress;
  /* AL Message Attribute */
  AL_MSG_ATTR attr;
  /* Destination LSAP */
  DLL_DSAP dsap;
  /* ECC (Encryption Coding Control) */
  DLL_ECC ecc;
  /* Rx status */
  AL_RX_STATUS rxStatus;
} AL_DATA_IND_PARAMS;

// *****************************************************************************
/* Meters And More AL Data request struct

  Summary:
    AL Data Request Parameters Structure.

   Description:
    Contains fields which define the AL Data Request input parameter.

  Remarks:
    Fields marked as Master Only are used in Master Node and ignored in
    Slave Nodes.
*/
typedef struct
{
  /* DATE-TIME for authenticated messages */
  uint64_t datetime;
  /* LMON of Meter used for authentication - MASTER ONLY */
  uint64_t lmon;
  /* Pointer to AL Data buffer (without message attribute) */
  uint8_t *apdu;
  /* Max length of the response - MASTER ONLY */
  uint16_t maxResponseLen;
  /* Number of time slots allocated in data request with Service_Class RC - MASTER ONLY */
  uint16_t timeSlotNum;
  /* Length of the AL data */
  uint16_t apduLen;
  /* Destination route - MASTER ONLY */
  ROUTING_ENTRY dstAddress;
  /* Service class - MASTER ONLY */
  SERVICE_CLASS serviceClass;
  /* AL Message Attribute */
  AL_MSG_ATTR attr;
  /* Destination LSAP */
  DLL_DSAP dsap;
  /* ECC (Encryption Coding Control) */
  DLL_ECC ecc;
} AL_DATA_REQUEST_PARAMS;

// *****************************************************************************
/* Meters And More AL Data request struct for Host Interface

  Summary:
    AL Data Request Parameters Structure used by Host Interface.

   Description:
    Contains fields which define the AL Data Request input parameter
    for Host Interface.

  Remarks:
    Only available when Host Interface is used.
*/
typedef struct
{
  /* DSAP */
  uint8_t dsap;
  /* Request ID */
  uint8_t reqID;
  /* Pointer to Data buffer */
  uint8_t *payload;
  /* Length of the data */
  uint16_t payloadLen;
} AL_DATA_REQUEST_PARAMS_HI;

// *****************************************************************************
/* Meters And More AL Data Confirm struct

  Summary:
    AL Data Confirm Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    AL Data Confirm Callback.

  Remarks:
    None.
*/
typedef struct
{
    /* Destination address */
    MAC_ADDRESS dstAddress;
    /* Destination LSAP */
    DLL_DSAP dsap;
    /* ECC (Encryption Coding Control) */
    DLL_ECC ecc;
    /* Tx status */
    AL_TX_STATUS txStatus;
} AL_DATA_CONFIRM_PARAMS;

// *****************************************************************************
/* Meters And More AL Event Indication struct

  Summary:
    AL Event Indication Parameters Structure.

   Description:
    Contains fields which define the information returned by the
    AL Event Indication Callback.

  Remarks:
    This Struct is only used in Master Node.
*/
typedef struct
{
    /* Event Identifier */
    AL_EVENT_ID eventId;
    /* First additional information */
    AL_EVENT_VALUE eventValue;
} AL_EVENT_IND_PARAMS;

// *****************************************************************************
/* Meters And More AL module Result

  Summary:
    Result of a Meters And More AL module interface operation.

  Description:
    Lists the possible results of Meters And More AL module operations.
*/
typedef enum
{
    AL_SUCCESS,
    AL_ERROR
} AL_RESULT;

// *****************************************************************************
/* Meters And More Application Layer module Initialization Data

  Summary:
    Defines the data required to initialize the Meters And More Application
    Layer module

  Description:
    Contains fields which define the information required by Application Layer
    module upon initialization:
    - The rate at which associated task is executed
    - The role of the Device, Master or Slave

  Remarks:
    None.
*/
typedef struct
{
  /* Application Layer task rate in milliseconds */
  uint8_t taskRateMs;
  /* Is master node (false in slave node) */
  bool isMaster;
  /* Initial value of AL_TX_RETRY_LIMIT */
  uint8_t txRetryLimit;
} AL_INIT;

// *****************************************************************************
/* Meters And More AL module Data Indication Function Pointer

  Summary:
    Pointer to a Meters And More AL module Data indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More AL
    module Data indication callback function. A client must register a pointer
	using the callback register function whose function signature (parameter and
	return value types) match the types specified by this function pointer in
	order to receive related event callbacks from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    indParams - Pointer to the object containing any data necessary to process the
             Data indication primitive.

  Returns:
    None.

  Example:
    <code>
    void APP_MyDataIndEventHandler( AL_DATA_IND_PARAMS *indParams )
    {
        if(indParams->apduLen > 0)
        {

        }
    }
    </code>
*/
typedef void ( *AL_DATA_IND_CALLBACK )( AL_DATA_IND_PARAMS *indParams );

// *****************************************************************************
/* Meters And More AL module Data Confirm Function Pointer

  Summary:
    Pointer to a Meters And More DLL module Data Confirm Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More AL
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
    void APP_MyDataCfmEventHandler( AL_DATA_CONFIRM_PARAMS *cfmParams )
    {
        switch(cfmParams->txStatus)
        {
            case AL_TX_STATUS_SUCCESS:
                break;
            default:
                break;
        }
    }
    </code>

  Remarks:
    - If the status field is AL_TX_STATUS_SUCCESS, it means that the data was
      transferred successfully.

    - Otherwise, it means that the data was not transferred successfully.

*/
typedef void ( *AL_DATA_CONFIRM_CALLBACK )( AL_DATA_CONFIRM_PARAMS *cfmParams );

// *****************************************************************************
/* Meters And More AL module Event Indication Function Pointer

  Summary:
    Pointer to a Meters And More AL module Event Indication Function Pointer.

  Description:
    This data type defines the required function signature for the Meters And More AL
    module Event Indication callback function. A client must register a pointer
    using the callback register function whose function signature (parameter and
    return value types) match the types specified by this function pointer in
    order to receive related event callbacks from the module.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    indParams - Pointer to the object containing any data necessary to process the
                Event Indication primitive.

  Returns:
    None.

  Example:
    <code>
    void APP_MyDatEventIndHandler( AL_EVENT_IND_PARAMS *eventParams )
    {
         eventParams->eventValue;
    }
    </code>
*/
typedef void ( *AL_EVENT_IND_CALLBACK )( AL_EVENT_IND_PARAMS *eventParams );

// *****************************************************************************
// *****************************************************************************
// Section: AL Interface Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    AL_RESULT AL_DataIndicationCallbackRegister (
        AL_DATA_IND_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More AL Data indication
    event handling function for the module to call back when the requested
    transmission has finished.

  Description:
    This function allows a client to register a Meters And More
    AL Data indication event handling function for the module to call back
    when a Meters And More AL data indication event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Rx_Ind_callback(AL_DATA_IND_PARAMS *indParams )
    {
        if (indParams->apduLen > 0){

        }
    }

    AL_DataIndicationCallbackRegister(APP_Rx_Ind_callback);
    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving notifications.
*/
AL_RESULT AL_DataIndicationCallbackRegister( AL_DATA_IND_CALLBACK callback );

// *****************************************************************************
/* Function:
    AL_RESULT AL_DataConfirmCallbackRegister (
        AL_DATA_CONFIRM_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More AL AL_Data.confirm
    event handling function for the module to call back when the requested
    transmission has finished.

  Description:
    This function allows a client to register a Meters And More
    AL AL_Data.confirm event handling function for the module to call back
    when a Meters And More AL data confirm event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Tx_Cfm_callback(AL_DATA_CONFIRM_PARAMS *cfmParams)
    {
        if (cfmParams->txStatus == AL_TX_STATUS_SUCCESS)
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
AL_RESULT AL_DataConfirmCallbackRegister(AL_DATA_CONFIRM_CALLBACK callback);

// *****************************************************************************
/* Function:
    AL_RESULT AL_EventIndicationCallbackRegister (
        AL_EVENT_IND_CALLBACK callback
    );

  Summary:
    Allows a client to set a Meters And More AL Data Event indication
    event handling function for the module to call back
    when AL generates or receives a new event.

  Description:
    This function allows a client to register a Meters And More
    AL Event indication event handling function for the module to call back
    when a Meters And More AL event occurs.

  Parameters:
    callback - Pointer to the callback function.

  Returns:
    None.

  Example:
    <code>
    void APP_Event_Ind_callback(AL_EVENT_IND_CALLBACK *indParams)
    {
        if (indParams->eventId == MAC_EVENT_ID_ACA){

        }
    }

    AL_EventIndicationCallbackRegister(APP_Event_Ind_callback);
    </code>

  Remarks:
    Callback can be set to a NULL pointer to stop receiving notifications.
    This Callback is only generated in Master Node. There is no need to set
    a handling function on Slave Nodes.
*/
AL_RESULT AL_EventIndicationCallbackRegister( AL_EVENT_IND_CALLBACK callback );

// *****************************************************************************
/* Function:
    void AL_DataRequest (
        AL_DATA_REQUEST_PARAMS *reqParams
    );

  Summary:
    AL Data request

  Description:
    Function that implements the AL Data request primitive

  Precondition:
    The low-level board initialization must have been completed and
    the module's initialization function must have been called before
    the system can call the tasks routine for any module.

  Parameters:
    drParams - Pointer to structure containing parameters related to AL Data request

  Returns:
    None.

  Example:
    <code>
    AL_DATA_REQUEST_PARAMS drParams;

    drParams.serviceClass = SERVICE_CLASS_RA;
    drParams.dstAddress.macAddress.address = dest_addressBuff;
    drParams.maxResponseLen = 128;
    drParams.timeSlotNum = 8;
    drParams.lmon = Node_Lmon_Counter;
    drParams.apdu = data;
    drParams.apduLen = 10;
	drParams.timestamp = 0x66FB3B80;

    AL_DataRequest(&drParams);
    </code>

  Remarks:
    Node_Info parameter needed for processing protected messages in AL are specific to each node.
	This information to be sent by concentrator for each data request placed. This is applicable
	only for Concentrator.
*/
void AL_DataRequest( AL_DATA_REQUEST_PARAMS *reqParams );

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ AL_Initialize (
      const SYS_MODULE_INDEX index,
      const SYS_MODULE_INIT * const init
    );

  Summary:
    Initializes the Meters And More AL module according to initialization
    parameters.

  Description:
    This routine initializes the Meters And More AL module.
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

    sysObjMetersandmore = AL_Initialize(0, (SYS_MODULE_INIT *)&init);
    </code>

  Remarks:
    This routine must be called before any other AL routine is called.
*/
SYS_MODULE_OBJ AL_Initialize (const SYS_MODULE_INDEX index, const SYS_MODULE_INIT * const init);

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ AL_Tasks (
        SYS_MODULE_OBJ object
    );

  Summary:
    Routine that maintains the state machine in the AL module.

  Description:
    Routine that performs the tasks necessary to maintain the state machine in
    the Meters And More AL module.

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

    sysObjMetersandmore = AL_Initialize(0, (SYS_MODULE_INIT *)&InitData);

    AL_Tasks(sysObjMetersandmore);
    </code>

  Remarks:
    None.
*/
void AL_Tasks( SYS_MODULE_OBJ object );

// *****************************************************************************
/* Function:
    SYS_STATUS AL_GetStatus(void);

  Summary:
    Gets the status of the AL module.

  Description:
    This function allows to retrieve AL module status.
    It must be used to ensure the module is Ready before start using it.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    Returns the status of the Meters And Mores AL module.
    - SYS_STATUS_UNINITIALIZED: AL module has not been initialized.
    - SYS_STATUS_BUSY: AL module is busy in the process of initialization.
    - SYS_STATUS_READY: AL module is ready to be used.

  Example:
    <code>
    case APP_AL_STATE_START:
    {
        if (AL_GetStatus() == AL_STATUS_READY)
        {
            app_ALData.state = APP_AL_STATE_RUNNING;
        }

        break;
    }
    </code>

  Remarks:
    None.
*/
SYS_STATUS AL_GetStatus(void);

// *****************************************************************************
/* Function:
    AL_RESULT AL_GetRequest (
        AL_IB_ATTRIBUTE attribute,
        uint16_t index,
        AL_IB_VALUE *ibValue
    );

  Summary:
    The AL_GetRequest primitive gets the value of an attribute in the
    AL or DLL layer Parameter Information Base (IB).

  Description:
    GetRequest primitive is used to get the value of an IB.
    Result is provided upon function call return, in the ibValue parameter.

  Precondition:
    None.

  Parameters:
    attribute - Identifier of the Attribute to retrieve value

    index - Index of element in case Attribute is a table
            Otherwise index must be set to '0'

    ibValue - Pointer to AL_IB_VALUE object where value will be returned

  Returns:
    Result of get operation as a AL_RESULT Enum.

  Example:
    <code>
    AL_RESULT result;
    AL_IB_VALUE value;
    result = AL_GetRequest(AL_AUTH_LMON_IB, 0, &value);
    if (result == AL_RESULT_SUCCESS)
    {

    }
    </code>

  Remarks:
    None.
*/
AL_RESULT AL_GetRequest(AL_IB_ATTRIBUTE attribute, uint16_t index,
                        AL_IB_VALUE *ibValue);

// *****************************************************************************
/* Function:
    AL_RESULT AL_SetRequest (
        AL_IB_ATTRIBUTE attribute,
        uint16_t index,
        const AL_IB_VALUE *ibValue
    );

  Summary:
    The AL_SetRequest primitive sets the value of an attribute in the
    AL or DLL layer Parameter Information Base (IB).

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

    ibValue - Pointer to AL_IB_VALUE object where value is contained

  Returns:
    Result of set operation as a AL_RESULT Enum.

  Example:
    <code>
    AL_RESULT result;
    const AL_IB_VALUE value;

    value.length = AL_LMON_LENGTH;
    memset(value.value, 0, AL_LMON_LENGTH);

    result = AL_SetRequest(AL_AUTH_LMON_IB, 0, &value);
    if (result == AL_RESULT_SUCCESS)
    {

    }
    </code>

  Remarks:
    None.
*/
AL_RESULT AL_SetRequest(AL_IB_ATTRIBUTE attribute, uint16_t index,
                        const AL_IB_VALUE *ibValue);

// *****************************************************************************
/* Function:
    AL_M_STATUS AL_DataRequestHI (
        AL_DATA_REQUEST_PARAMS_HI *reqParams
    );

  Summary:
    AL Data request with Host Interface format.

  Description:
    Function that implements the AL Data request using Host Interface format.

  Precondition:
    None.

  Parameters:
    reqParams - Pointer to structure containing parameters related to Data request.

  Returns:
    None.

  Example:
    <code>
    AL_DATA_REQUEST_PARAMS_HI drParams;

    drParams.dsap = 0x00;
    drParams.reqID = 0xAA;
    drParams.payload = dataBuf;
    drParams.payloadLen = dataBufLen;

    AL_DataRequestHI(&drParams);
    </code>

  Remarks:
    Only available when Host Interface is used.
*/
void AL_DataRequestHI(AL_DATA_REQUEST_PARAMS_HI *reqParams);

// *****************************************************************************
/* Function:
    AL_RESULT AL_PerformECB (
        uint8_t *dataIn,
        uint8_t dataLen,
        uint8_t *dataOut,
        uint8_t *key,
        uint8_t keyLen
    );

  Summary:
    The AL_PerformECB primitive provides AES-ECB Encryption capabilities.

  Description:
    This function is provided for the upper layers to take advantage of
    encryption capabilities of AL.
    It performs an AES-ECB Encryption to a data buffer using provided input
    parameters and returns the encrypted data in the provided output parameter.

  Precondition:
    None.

  Parameters:
    dataIn - Pointer to data to be encrypted
    dataLen - Length of data buffer to be encrypted (in bytes)
    dataOut - Pointer to buffer where encrypted data will be written
    key - Pointer to key to be used for encryption
    keyLen - Length of key to be used for encryption (in bytes)

  Returns:
    AL_SUCCESS if encryption succeeds. AL_ERROR otherwise.

  Example:
    <code>
    AL_RESULT result;
    uint8_t rawData[64];
    uint8_t encryptedData[64];
    uint8_t aesKey[16];

    memcpy(rawData, appData, sizeof(rawData));
    memcpy(aesKey, appKey, sizeof(aesKey));

    result = AL_PerformECB(rawData, 64, encryptedData, aesKey, 16);
    if (result == AL_SUCCESS)
    {
        sendData(encryptedData);
    }
    </code>

  Remarks:
    None.
*/
AL_RESULT AL_PerformECB(uint8_t *dataIn, uint8_t dataLen, uint8_t *dataOut, uint8_t *key, uint8_t keyLen);

#ifdef __cplusplus
}
#endif

#endif /* AL_H */

/* *****************************************************************************
 End of File
 */
