/*******************************************************************************
  Meters and More Host Interface Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    MMHI_definitions.h

  Summary:
    Meters and More Host Interface local implementation.

  Description:
    This file contains the definitions required by the MMHI interface.
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

#ifndef MMHI_DEFINITIONS_H    /* Guard against multiple inclusion */
#define MMHI_DEFINITIONS_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "system/ports/sys_ports.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END


typedef size_t (*MMHI_PLIB_READ)(uint8_t* pRdBuffer, const size_t size );
typedef void (*MMHI_PLIB_READCALLBACKREGISTER)(void* callback, uintptr_t context);

typedef size_t (*MMHI_PLIB_WRITE)(uint8_t* pWrBuffer, const size_t size );
typedef void (*MMHI_PLIB_WRITECALLBACKREGISTER)(void* callback, uintptr_t context);

typedef uint32_t (*MMHI_PLIB_ERROR_GET)( void );

typedef struct
{
    MMHI_PLIB_READ                          readFn;
    MMHI_PLIB_READCALLBACKREGISTER          readCallbackRegister;

    MMHI_PLIB_WRITE                         writeFn;
    MMHI_PLIB_WRITECALLBACKREGISTER         writeCallbackRegister;

    MMHI_PLIB_ERROR_GET                     errorGet;

} MMHI_PLIB_INTERFACE;

typedef struct
{
    const MMHI_PLIB_INTERFACE*              uartPLIB;

} MMHI_INIT_DATA;

// *****************************************************************************
/* Meters and More Host Interface Handle

  Summary:
    Handle to an opened M&M host interface.

  Description:
    This handle identifies the open instance of a host interface.  It must be
    passed to all other MMHI routines (except the initialization, deinitialization,
    or power routines) to identify the caller.

  Remarks:
    Every application or module that wants to use a driver must first call
    the interface's open routine.  This is the only routine that is absolutely
    required for MMHI.

    If MMHI is unable to allow an additional module to use it, it must then
    return the special value MMHI_HANDLE_INVALID.  Callers should check the
    handle returned for this value to ensure this value was not returned before
    attempting to call any other MMHI routines using the handle.
*/

typedef uintptr_t MMHI_HANDLE;


// *****************************************************************************
/* Invalid Meters and More Host Interface Handle

 Summary:
    Invalid M&M Host Interface Handle

 Description:
    If the MMHI is unable to allow an additional module to use it, it must then
    return the special value MMHI_HANDLE_INVALID.  Callers should check the
    handle returned for this value to ensure this value was not returned before
    attempting to call any other MMHI routines using the handle.

 Remarks:
    None.
*/

#define MMHI_HANDLE_INVALID  (((MMHI_HANDLE) -1))

// *****************************************************************************
/* Meters And More HI Command Codes

  Summary:
    All command codes available for EUT

  Description:
    Each command frame exchanged between the host controller and the EUT carries
    a command, which is identified by a unique command code
*/
typedef enum
{
    MMHI_CMD_BIO_RESET_REQ = 0x3C,
    MMHI_CMD_BIO_RESET_CFM = 0x3D,
    MMHI_CMD_BIO_RESET_IND = 0x3E,
    MMHI_CMD_MIB_WRITE_REQ = 0x08,
    MMHI_CMD_MIB_WRITE_CFM = 0x09,
    MMHI_CMD_MIB_WRITE_NCFM = 0x0B,
    MMHI_CMD_MIB_WRITE_IND = 0x0A,
    MMHI_CMD_MIB_READ_REQ = 0x0C,
    MMHI_CMD_MIB_READ_CFM = 0x0D,
    MMHI_CMD_MIB_READ_NCFM = 0x0F,
    MMHI_CMD_SLAVE_DATA_REQ = 0x24,
    MMHI_CMD_SLAVE_DATA_CFM = 0x25,
    MMHI_CMD_SLAVE_DATA_NCFM = 0x27,
    MMHI_CMD_SLAVE_DATA_IND = 0x26,
    MMHI_CMD_MASTER_DATA_REQ = 0x28,
    MMHI_CMD_MASTER_DATA_CFM = 0x29,
    MMHI_CMD_MASTER_DATA_NCFM = 0x2B,
    MMHI_CMD_MASTER_DATA_IND = 0x2A,
    MMHI_CMD_HI_PING_REQ = 0x2C,
    MMHI_CMD_HI_PING_CFM = 0x2D,
    MMHI_CMD_HI_ERROR_IND = 0x36,
    /* Manufacturer commands */
    MMHI_CMD_PLC_PHY_DATA_REQ = 0x48,
    MMHI_CMD_PLC_PHY_DATA_CFM = 0x49,
    MMHI_CMD_PLC_PHY_DATA_NCFM = 0x4B,
    MMHI_CMD_PLC_PHY_DATA_IND = 0x4A,
    MMHI_CMD_INVALID = 0xFF
} MMHI_COMMAND;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus

    }

#endif
// DOM-IGNORE-END

#endif /* MMHI_DEFINITIONS_H */