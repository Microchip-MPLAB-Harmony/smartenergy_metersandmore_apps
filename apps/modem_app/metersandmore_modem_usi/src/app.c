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

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void lAPP_BlinkLedTimerCallback (uintptr_t context)
{
    appData.tmrBlinkLedExpired = true;
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
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Refresh WDG */
    CLEAR_WATCHDOG();

    /* Blinking LED management */
    if (appData.tmrBlinkLedExpired)
    {
        appData.tmrBlinkLedExpired = false;
        BLINK_LED_Toggle();
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

            appData.state = APP_STATE_IDLE;
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
