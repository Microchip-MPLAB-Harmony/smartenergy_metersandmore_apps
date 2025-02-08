/*******************************************************************************
  Interface definition of Meters And More HI (Host Interface) module.

  Company:
    Microchip Technology Inc.

  File Name:
    mmhi_mib.h

  Summary:
    Interface MIB definition of Meters And More HI (Host Interface) module.

  Description:
    This file defines the MIB interface for the Meters And More HI (Host Interface)
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

#ifndef MMHI_MIB_H
#define MMHI_MIB_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
#include "system/system.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

#define MMHI_MIB_MAX_LENGTH_DATA      32U

#define MMHI_MIB_FW_VERSION_MAJOR     _UINT8_(0x01U)
#define MMHI_MIB_FW_VERSION_MINOR     _UINT8_(0x00U)

#define MMHI_MIB_TIME_TEL             _UINT16_(20U)
#define MMHI_MIB_TIME_DELAY           _UINT16_(1000U)
#define MMHI_MIB_TIME_ICDELAY         _UINT16_(0U)
#define MMHI_MIB_TIME_TC              _UINT16_(250U)
#define MMHI_MIB_TIME_TCT             _UINT8_(0xFFU)

/* Status Message: Status Mask field description */
#define HI_SMSK_MAC_CFG_MIB_Pos                           _UINT16_(3U)
#define HI_SMSK_MAC_CFG_MIB_Msk                           (_UINT16_(0x1U) << HI_SMSK_MAC_CFG_MIB_Pos)
#define HI_SMSK_MAC_CFG_MIB(value)                        (HI_SMSK_MAC_CFG_MIB_Msk & (_UINT16_(value) << HI_SMSK_MAC_CFG_MIB_Pos))
#define   HI_SMSK_MAC_CFG_MIB_CONFIGURED_Val              _UINT16_(0x0U)
#define   HI_SMSK_MAC_CFG_MIB_DEFAULT_Val                 _UINT16_(0x1U)
#define HI_SMSK_FW_RELEASE_MIB_Pos                        _UINT16_(4U)
#define HI_SMSK_FW_RELEASE_MIB_Msk                        (_UINT16_(0x1U) << HI_SMSK_FW_RELEASE_MIB_Pos)
#define HI_SMSK_FW_RELEASE_MIB(value)                     (HI_SMSK_FW_RELEASE_MIB_Msk & (_UINT16_(value) << HI_SMSK_FW_RELEASE_MIB_Pos))
#define   HI_SMSK_FW_RELEASE_MIB_CONFIGURED_Val           _UINT16_(0x0U)
#define   HI_SMSK_FW_RELEASE_MIB_DEFAULT_Val              _UINT16_(0x1U)
#define HI_SMSK_MNF_DATA_MIB_Pos                          _UINT16_(6U)
#define HI_SMSK_MNF_DATA_MIB_Msk                          (_UINT16_(0x1U) << HI_SMSK_MNF_DATA_MIB_Pos)
#define HI_SMSK_MNF_DATA_MIB(value)                       (HI_SMSK_MNF_DATA_MIB_Msk & (_UINT16_(value) << HI_SMSK_MNF_DATA_MIB_Pos))
#define   HI_SMSK_MNF_DATA_MIB_CONFIGURED_Val             _UINT16_(0x0U)
#define   HI_SMSK_MNF_DATA_MIB_DEFAULT_Val                _UINT16_(0x1U)
#define HI_SMSK_SCA_MIB_Pos                               _UINT16_(7U)
#define HI_SMSK_SCA_MIB_Msk                               (_UINT16_(0x1U) << HI_SMSK_SCA_MIB_Pos)
#define HI_SMSK_SCA_MIB(value)                            (HI_SMSK_SCA_MIB_Msk & (_UINT16_(value) << HI_SMSK_SCA_MIB_Pos))
#define   HI_SMSK_SCA_MIB_CONFIGURED_Val                  _UINT16_(0x0U)
#define   HI_SMSK_SCA_MIB_DEFAULT_Val                     _UINT16_(0x1U)
#define HI_SMSK_ENCRYPTION_KEYS_MIB_Pos                   _UINT16_(8U)
#define HI_SMSK_ENCRYPTION_KEYS_MIB_Msk                   (_UINT16_(0x1U) << HI_SMSK_ENCRYPTION_KEYS_MIB_Pos)
#define HI_SMSK_ENCRYPTION_KEYS_MIB(value)                (HI_SMSK_ENCRYPTION_KEYS_MIB_Msk & (_UINT16_(value) << HI_SMSK_ENCRYPTION_KEYS_MIB_Pos))
#define   HI_SMSK_ENCRYPTION_KEYS_MIB_CONFIGURED_Val      _UINT16_(0x0U)
#define   HI_SMSK_ENCRYPTION_KEYS_MIB_DEFAULT_Val         _UINT16_(0x1U)
#define HI_SMSK_SECURITY_FLAGS_MIB_Pos                    _UINT16_(9U)
#define HI_SMSK_SECURITY_FLAGS_MIB_Msk                    (_UINT16_(0x1U) << HI_SMSK_SECURITY_FLAGS_MIB_Pos)
#define HI_SMSK_SECURITY_FLAGS_MIB(value)                 (HI_SMSK_SECURITY_FLAGS_MIB_Msk & (_UINT16_(value) << HI_SMSK_SECURITY_FLAGS_MIB_Pos))
#define   HI_SMSK_SECURITY_FLAGS_MIB_CONFIGURED_Val       _UINT16_(0x0U)
#define   HI_SMSK_SECURITY_FLAGS_MIB_DEFAULT_Val          _UINT16_(0x1U)
#define HI_SMSK_LMON_MIB_Pos                              _UINT16_(10U)
#define HI_SMSK_LMON_MIB_Msk                              (_UINT16_(0x1U) << HI_SMSK_LMON_MIB_Pos)
#define HI_SMSK_LMON_MIB(value)                           (HI_SMSK_LMON_MIB_Msk & (_UINT16_(value) << HI_SMSK_LMON_MIB_Pos))
#define   HI_SMSK_LMON_MIB_CONFIGURED_Val                 _UINT16_(0x0U)
#define   HI_SMSK_LMON_MIB_DEFAULT_Val                    _UINT16_(0x1U)
#define HI_SMSK_TIMING_PARAMS_MIB_Pos                     _UINT16_(12U)
#define HI_SMSK_TIMING_PARAMS_MIB_Msk                     (_UINT16_(0x1U) << HI_SMSK_TIMING_PARAMS_MIB_Pos)
#define HI_SMSK_TIMING_PARAMS_MIB(value)                  (HI_SMSK_TIMING_PARAMS_MIB_Msk & (_UINT16_(value) << HI_SMSK_TIMING_PARAMS_MIB_Pos))
#define   HI_SMSK_TIMING_PARAMS_MIB_CONFIGURED_Val        _UINT16_(0x0U)
#define   HI_SMSK_TIMING_PARAMS_MIB_DEFAULT_Val           _UINT16_(0x1U)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

/* Meters And More HI MIB Result

  Summary:
    Result of a Meters And More host MIB operation.

  Description:
    Identifies the result of certain MMHI MIB operations.
*/

typedef enum
{
    // Operation completed with success.
    MMHI_MIB_SUCCESS,

    // Invalid handle or operation failed.
    MMHI_MIB_ERROR,

} MMHI_MIB_RESULT;

typedef enum
{
    MIB_MIB_ID_MAC_CONFIG = 3,
    MIB_MIB_ID_FW_RELEASE = 4,
    MIB_MIB_ID_MANUF_DATA = 6,
    MIB_MIB_ID_LOGICAL_ADDRESS = 7,
    MIB_MIB_ID_ENCRYPTION_KEYS = 8,
    MIB_MIB_ID_SECURITY_FLAGS = 9,
    MIB_MIB_ID_LMON = 10,
    MIB_MIB_ID_TIMING = 12,
} MMHI_MIB_INDEX;

// *****************************************************************************
/* Meters And More HI MAC reception modes

  Summary:
    RX MAC modes

  Description:
    Identifies the mode of the MAC reception
*/
typedef enum
{
    MIB_RX_DISABLED = 0,
    MIB_RX_NORMAL,
    MIB_RX_MAC_SNIFFFER,
    MIB_RX_PHY_SNIFFFER
} MMHI_MIB_MAC_RX;

/* Meters And More HI MAC transmission modes

  Summary:
    TX MAC modes

  Description:
    Identifies the mode of the MAC transmission
*/
typedef enum
{
    MIB_TX_DISABLED = 0,
    MIB_TX_NORMAL,
} MMHI_MIB_MAC_TX;

#pragma pack(push,1)
// *****************************************************************************
/* Meters And More MAC layer configuration

  Summary:
    Defines the data required to configure the MAC layer

  Remarks:
    None.
*/
typedef struct
{
    /* MAC RX mode */
    MMHI_MIB_MAC_RX rxMode;
    /* MAC TX mode */
    MMHI_MIB_MAC_TX txMode;

} MMHI_MIB_MAC_CONFIG;

// *****************************************************************************
/* Meters And More FW release information

  Summary:
    Defines the Firmware release number

  Remarks:
    None.
*/
typedef struct
{
    /* Customer specific. */
    uint16_t modemHash;
    /* RESERVED */
    uint8_t meterApp1Major;
    /* RESERVED */
    uint8_t meterApp1Mminor;
    /* RESERVED */
    uint8_t meterApp2Major;
    /* RESERVED */
    uint8_t meterApp2Mminor;
    /* Modem Version Major */
    uint8_t modemMajor;
    /* Modem Version Minor */
    uint8_t modemMinor;

} MMHI_MIB_FW_VERSION;

// *****************************************************************************
/* Meters And More MAC absolute address

  Summary:
    Defines the MAC absolute address (6 bytes)

  Remarks:
    None.
*/
typedef struct
{
    uint8_t value[6];

} MMHI_MIB_ADDRESS;

// *****************************************************************************
/* Meters And More Manufacturer information

  Summary:
    Defines the data provided by the manufacturer

  Remarks:
    None.
*/
typedef struct
{
    /* Manufacturer custom serial number of device */
    uint8_t serialNumber[16];
    /* Absolute Address. little endian */
    MMHI_MIB_ADDRESS aca;

} MMHI_MIB_MANUFACTURER_DATA;

// *****************************************************************************
/* Meters And More Logical Address information

  Summary:
    Defines the meter addressing parameters

  Remarks:
    None.
*/
typedef struct
{
    /* Section address. little endian */
    uint8_t section[3];
    /* Subsection address */
    uint8_t subSection;
    /* Node address */
    uint8_t node;
    /* Length of Section address (this is fixed to 3) */
    uint8_t sectionLength;

} MMHI_MIB_LOGICAL_ADDRESS;

// *****************************************************************************
/* Meters And More Internal timing parameters

  Summary:
    Defines the meter timeouts

  Remarks:
    None.
*/
typedef struct
{
    /* Elaboration time */
    uint16_t tel;
    /* Added Delay */
    uint16_t delay;
    /* Added Delay for IC */
    uint16_t icDelay;
    /* Time slot, Default 250 ms */
    uint16_t tc;
    /* silencing level of the node */
    uint8_t tct;

} MMHI_MIB_TIMING_PARAMETERS;

// *****************************************************************************
/* Meters And More Encryption Keys MIB

  Summary:
    Defines the Write and Read Keys for encryption

  Remarks:
    None.
*/
typedef struct
{
    /* Write Key */
    uint8_t keyW[16];
    /* Read Key */
    uint8_t keyR[16];

} MMHI_MIB_ENCRYPTION_KEYS;

// *****************************************************************************
/* Meters And More Security Flags MIB

  Summary:
    Defines the flags to enable/disable security options

  Remarks:
    None.
*/
typedef struct
{
    /* Flags */
    uint8_t flags;

} MMHI_MIB_SECURITY_FLAGS;

// *****************************************************************************
/* Meters And More LMON MIB

  Summary:
    Sets the LMON to use in Slave Node

  Remarks:
    None.
*/
typedef struct
{
    /* LMON */
    uint8_t lmon[8];

} MMHI_MIB_LMON;

// *****************************************************************************
/* Meters And More Management Information Base (MIB) data

  Summary:
    Defines the MIB data

  Remarks:
    None.
*/
typedef struct
{
    MMHI_MIB_MAC_CONFIG macConfig;
    MMHI_MIB_FW_VERSION fwVersion;
    MMHI_MIB_MANUFACTURER_DATA manufacturer;
    MMHI_MIB_LOGICAL_ADDRESS address;
    MMHI_MIB_ENCRYPTION_KEYS securityKeys;
    MMHI_MIB_SECURITY_FLAGS securityFlags;
    MMHI_MIB_LMON securityLmon;
    MMHI_MIB_TIMING_PARAMETERS timing;

} MMHI_MIB_DB;

// *****************************************************************************
/* MMHI Get MIB Data

   Summary:
    Defines the parameters for the MMHI Get MIB Confirm event handler function.

   Description:
    The structure contains the fields reported by the MMHI Get MIB Data.

   Remarks:
    None.
*/
typedef struct
{
    /* The length of the value of the MIB read */
    uint8_t dataLength;

    /* The value of the MIB read */
    uint8_t dataValue[MMHI_MIB_MAX_LENGTH_DATA];

} MMHI_MIB_DATA;

#pragma pack(pop)

typedef void ( *MMHI_MIB_WRITE_IND_CALLBACK )( MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData );

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

void MMHI_MIB_Initialize(void);
uint16_t MMHI_MIB_GetStatus(void);
MMHI_RESULT MMHI_MIB_Get(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData);
MMHI_RESULT MMHI_MIB_Set(MMHI_MIB_INDEX mibIndex, MMHI_MIB_DATA* pData, bool indEnable);
void MMHI_MIB_WriteIndCallbackRegister(MMHI_MIB_WRITE_IND_CALLBACK callback);
void MMHI_MIB_Tasks(void);

#ifdef __cplusplus
 }
#endif

#endif // #ifndef MMHI_MIB_H
/*******************************************************************************
 End of File
*/
