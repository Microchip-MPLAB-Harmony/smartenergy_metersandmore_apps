/*******************************************************************************
  Meters&More AL Serialization Header File

  Company:
    Microchip Technology Inc.

  File Name:
    al_serial.h

  Summary:
    Meters&More AL Serialization API Header File.

  Description:
    The G3 AL Serialization allows to serialize the AL and LBP API through
    USI interface in order to run the application on an external device. This
    file contains definitions of the API of G3 AL Serialization.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2025, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#ifndef AL_SERIAL_H
#define AL_SERIAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "al.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: AL Serial Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ AL_SERIAL_Initialize(const SYS_MODULE_INDEX index);

  Summary:
    Initializes the AL Serialization module for the specified index.

  Description:
    This routine initializes the AL Serialization module for the specified
    index.

  Precondition:
    None.

  Parameters:
    index - Identifier for the instance to be initialized (single instance
            allowed)

  Returns:
    If successful, returns a valid module instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    SYS_MODULE_OBJ sysObjAdpSerial;

    sysObjAdpSerial = AL_SERIAL_Initialize(MM_AL_SERIAL_INDEX_0);
    if (sysObjAdpSerial == SYS_MODULE_OBJ_INVALID)
    {

    }
    </code>

  Remarks:
    This routine must be called before any other AL Serialization routine is
    called.
*/
SYS_MODULE_OBJ AL_SERIAL_Initialize(const SYS_MODULE_INDEX index);

// *****************************************************************************
/* Function:
    void AL_SERIAL_Tasks(SYS_MODULE_OBJ object);

  Summary:
    Maintains AL Serialization State Machine.

  Description:
    This routine maintains AL Serialization State Machine.

  Precondition:
    The AL_SERIAL_Initialize routine must have been called to obtain a valid
    system object.

  Parameters:
    object - System object handle, returned from the AL_SERIAL_Initialize
             routine.

  Returns:
    None.

  Example:
    <code>
    SYS_MODULE_OBJ sysObjAdpSerial;

    while (true)
    {
        AL_SERIAL_Tasks(sysObjAdpSerial);
    }
    </code>

  Remarks:
    This function is normally not called directly by an application. It is
    called by the system's tasks routine (SYS_Tasks).
*/
void AL_SERIAL_Tasks(SYS_MODULE_OBJ object);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // #ifndef AL_SERIAL_H
