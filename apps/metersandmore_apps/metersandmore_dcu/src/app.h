/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* LED timings */
#define LED_BLINK_RATE_MS                         500
#define LED_PLC_RX_MSG_RATE_MS                    50

/* Timeout to advance App State */
#define STATE_TIMEOUT_MS                          5000

/* Routing Table Size */
#define ROUTING_TABLE_SIZE                        20

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_STATE_INIT=0,
    APP_STATE_WAIT_AL_READY,
    APP_STATE_CLEAR_NODE_LIST,
    APP_STATE_SEND_TCT_BROADCAST,
    APP_STATE_WAIT_TCT_SENT,
    APP_STATE_SEND_ADDR_REQ,
    APP_STATE_WAIT_ADDR_RESP,
    APP_STATE_SEND_TCT_SILENCE_NODES,
    APP_STATE_WAIT_TCT_SILENCE_NODES_SENT,
    APP_STATE_SEND_TCT_BROADCAST_REPEATER,
    APP_STATE_WAIT_TCT_SENT_REPEATER,
    APP_STATE_SEND_REQADDR_REQ,
    APP_STATE_WAIT_REQADDR_RESP,
    APP_STATE_SEND_PLAIN_READ_REQ,
    APP_STATE_WAIT_PLAIN_READ_RESP,
    APP_STATE_SEND_ENCRYPTED_READ_REQ,
    APP_STATE_WAIT_ENCRYPTED_READ_RESP,
    APP_STATE_IDLE,

} APP_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    SYS_TIME_HANDLE tmrBlinkLedHandle;

    SYS_TIME_HANDLE tmrPlcIndLedHandle;

    SYS_TIME_HANDLE tmrStateTimeoutHandle;

    ROUTING_ENTRY routingTable[ROUTING_TABLE_SIZE];

    uint64_t lmonTable[ROUTING_TABLE_SIZE];

    AL_IB_VALUE alIB;

    uint8_t numTCTBroadcastSent;
    
    uint8_t numTCTSilenceSent;
    
    uint8_t numTCTBroadcastRepeaterSent;

    uint8_t numFoundNodes;
    
    uint8_t numFoundNodesNew;

    uint8_t numReqAddrSent;

    uint8_t numReadPlainSent;

    uint8_t numReadEncryptedSent;
    
    bool addressReqFinished;

    bool tmrBlinkLedExpired;

    bool tmrPlcIndLedExpired;

    bool tmrStateTimeoutExpired;

    bool lmonMismatchReceived;

    APP_STATES state;

} APP_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_H */

/*******************************************************************************
 End of File
 */

