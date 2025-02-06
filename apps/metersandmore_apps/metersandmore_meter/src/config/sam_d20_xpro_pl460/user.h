/*******************************************************************************
  User Configuration Header

  File Name:
    user.h

  Summary:
    Build-time configuration header for the user defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    It only provides macro definitions for build-time configuration options

*******************************************************************************/

#ifndef USER_H
#define USER_H

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: User Configuration macros
// *****************************************************************************
// *****************************************************************************

/* Macros for blinking LED */
#define BLINK_LED_On()           BSP_LED0_On()
#define BLINK_LED_Off()          BSP_LED0_Off()
#define BLINK_LED_Toggle()       BSP_LED0_Toggle()

/* Macros for PLC Indication LED */
#define PLC_IND_LED_On()
#define PLC_IND_LED_Off()
#define PLC_IND_LED_Toggle()

/* Macro to clear watchdog */
#define CLEAR_WATCHDOG()         WDT_Clear()

/* Use this macro to configure MAC address (ACA) automatically from UniqueID */
#define CONFIG_ACA_AUTO_SAMD20

/* Use this macro to set a fixed MAC address (ACA) */
//#define CONFIG_ACA_FIXED         {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}

/* Macros to configure AL encryption keys */
#define CONFIG_AL_KEY_K1         {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}
#define CONFIG_AL_KEY_K2         {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // USER_H
/*******************************************************************************
 End of File
*/
