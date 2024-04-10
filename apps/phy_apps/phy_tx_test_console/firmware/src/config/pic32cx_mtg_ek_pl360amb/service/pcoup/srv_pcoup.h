/*******************************************************************************
  PLC PHY Coupling Service Library Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    srv_pcoup.h

  Summary
    PLC PHY Coupling service library interface.

  Description
    The Microchip G3-PLC and PRIME implementations include default PHY layer
    configuration values optimized for the Evaluation Kits. With the help of
    the PHY Calibration Tool it is possible to obtain the optimal configuration
    values for the customer's hardware implementation. Refer to the online
    documentation for more details about the available configuration values and
    their purpose.

  Remarks:
    This service provides the required information to be included on PLC
    projects for PL360/PL460 in order to apply the custom calibration.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*
Copyright (C) 2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
// DOM-IGNORE-END

#ifndef SRV_PCOUP_H    // Guards against multiple inclusion
#define SRV_PCOUP_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "driver/plc/phy/drv_plc_phy.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

/* Default branch of the PLC transmission coupling */
#define SRV_PCOUP_DEFAULT_BRANCH                 SRV_PLC_PCOUP_MAIN_BRANCH

/* PLC PHY Coupling parameters for Main branch */
#define SRV_PCOUP_RMS_HIGH_TBL                   {324, 293, 267, 242, 218, 194, 172, 153}
#define SRV_PCOUP_RMS_VLOW_TBL                   {2432, 2172, 1932, 1714, 1515, 1342, 1189, 1052}
#define SRV_PCOUP_THRS_HIGH_TBL                  {0, 0, 0, 0, 0, 0, 0, 0, 274, 247, 225, 205, 184, 164, 146, 130}
#define SRV_PCOUP_THRS_VLOW_TBL                  {0, 0, 0, 0, 0, 0, 0, 0, 5579, 4972, 4414, 3920, 3477, 3079, 2738, 2433}
#define SRV_PCOUP_DACC_TBL                       {0x0UL, 0x2120UL, 0x73fUL, 0x3f3fUL, 0x333UL, 0x0UL, \
                                                 0xffff00ffUL, 0x17171717UL, 0x2020UL, 0x44UL, 0xfd20001UL, 0x355UL, \
                                                 0xf000000UL, 0x1020f0UL, 0x355UL, 0xf000000UL, 0x1020ffUL}
#define SRV_PCOUP_GAIN_HIGH_TBL                  {25, 12, 50}
#define SRV_PCOUP_GAIN_VLOW_TBL                  {228, 114, 256}
#define SRV_PCOUP_NUM_TX_LEVELS                  8
#define SRV_PCOUP_LINE_DRV_CONF                  0

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* PLC PHY Coupling Branch definitions

 Summary:
    List of possible transmission branches.

 Description:
    This type defines the possible values of PLC transmission coupling branches.

 Remarks:
    None.
*/

typedef enum
{
    /* Main Transmission Branch */
    SRV_PLC_PCOUP_MAIN_BRANCH,

} SRV_PLC_PCOUP_BRANCH;

// *****************************************************************************
/* PLC PHY Coupling data

  Summary:
    PLC PHY Coupling data.

  Description:
    This structure contains all the data required to set the PLC PHY Coupling
    parameters, for a specific transmission branch (associated to a G3-PLC
    PHY band).

  Remarks:
    Equalization coefficients are not stored in the structure, just pointers to
    arrays were they are actually stored. This allows to use the same type for
    different G3-PLC PHY bands.
*/

typedef struct
{
    /* Target RMS values in HIGH mode for dynamic Tx gain */
    uint32_t                         rmsHigh[8];

    /* Target RMS values in VLOW mode for dynamic Tx gain */
    uint32_t                         rmsVLow[8];

    /* Threshold RMS values in HIGH mode for dynamic Tx mode */
    uint32_t                         thrsHigh[16];

    /* Threshold RMS values in VLOW mode for dynamic Tx mode */
    uint32_t                         thrsVLow[16];

    /* Values for configuration of PLC DACC peripheral, according to hardware
       coupling design and PLC device (PL360/PL460) */
    uint32_t                         daccTable[17];

    /* Tx gain values for HIGH mode [HIGH_INI, HIGH_MIN, HIGH_MAX] */
    uint16_t                         gainHigh[3];

    /* Tx gain values for VLOW mode [VLOW_INI, VLOW_MIN, VLOW_MAX] */
    uint16_t                         gainVLow[3];

    /* Number of Tx attenuation levels (3 dB step) supporting dynamic Tx mode */
    uint8_t                          numTxLevels;

    /* Configuration of the PLC Tx Line Driver, according to hardware coupling
       design and PLC device (PL360/PL460) */
    uint8_t                          lineDrvConf;

} SRV_PLC_PCOUP_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Service Interface Functions
// *****************************************************************************
// *****************************************************************************

/***************************************************************************
  Function:
    SRV_PLC_PCOUP_DATA * SRV_PCOUP_Get_Config(SRV_PLC_PCOUP_BRANCH branch)

  Summary:
    Get the PLC PHY Coupling parameters for the specified transmission branch.

  Description:
    This function allows to get the PLC PHY Coupling parameters for the
    specified transmission branch. These parameters can be sent to the PLC
    device through PLC Driver PIB interface (DRV_PLC_PHY_PIBSet).

  Precondition:
    None.

  Parameters:
    branch          - Transmission branch for which the parameters are requested

  Returns:
    - Pointer PLC PHY Coupling parameters
      - if branch parameter is valid
    - NULL
      - if branch parameter is not valid

  Example:
    <code>
    SRV_PLC_PCOUP_DATA *pCoupValues;

    pCoupValues = SRV_PCOUP_Get_Config(SRV_PLC_PCOUP_MAIN_BRANCH);
    </code>

  Remarks:
    If SRV_PCOUP_Set_Config is used to set the PLC PHY Coupling parameters,
    this function is not needed.
  ***************************************************************************/

SRV_PLC_PCOUP_DATA * SRV_PCOUP_Get_Config(SRV_PLC_PCOUP_BRANCH branch);

/***************************************************************************
  Function:
    bool SRV_PCOUP_Set_Config(DRV_HANDLE handle, SRV_PLC_PCOUP_BRANCH branch);

  Summary:
    Set the PLC PHY Coupling parameters for the specified transmission branch.

  Description:
    This function allows to set the PLC PHY Coupling parameters for the
    specified transmission branch, using the PLC Driver PIB
    interface (DRV_PLC_PHY_PIBSet).

  Precondition:
    DRV_PLC_PHY_Open must have been called to obtain a valid
    opened device handle.

  Parameters:
    handle  - A valid instance handle, returned from DRV_PLC_PHY_Open
    branch  - Transmission branch for which the parameters will be set

  Returns:
    - true
      - Successful configuration
    - false
      - if branch parameter is not valid
      - if there is an error when calling DRV_PLC_PHY_PIBSet

  Example:
    <code>
    bool result;

    result = SRV_PCOUP_Set_Config(handle, SRV_PLC_PCOUP_MAIN_BRANCH);
    </code>

  Remarks:
    None.
  ***************************************************************************/

bool SRV_PCOUP_Set_Config(DRV_HANDLE handle, SRV_PLC_PCOUP_BRANCH branch);

/***************************************************************************
  Function:
    SRV_PLC_PCOUP_BRANCH SRV_PCOUP_Get_Default_Branch( void )

  Summary:
    Get the default branch of the PLC transmission coupling.

  Description:
    This function allows to get the tranmission branch used by default.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    Default transmission branch.

  Example:
    <code>
    SRV_PLC_PCOUP_BRANCH plcDefaultBranch;

    plcDefaultBranch = SRV_PCOUP_Get_Default_Branch();
    SRV_PCOUP_Set_Config(plcDefaultBranch);
    </code>

  Remarks:
    None.
  ***************************************************************************/

SRV_PLC_PCOUP_BRANCH SRV_PCOUP_Get_Default_Branch( void );

/***************************************************************************
  Function:
    uint8_t SRV_PCOUP_Get_Phy_Band(SRV_PLC_PCOUP_BRANCH branch)

  Summary:
    Get the G3-PLC PHY band associated to the specified transmission branch.

  Description:
    This function allows to get the G3-PLC PHY band associated to the
    specified transmission branch.

  Precondition:
    None.

  Parameters:
    branch         - Transmission branch from which the PHY band is requested

  Returns:
    G3-PLC PHY band associated to the specified transmission branch
    (see drv_plc_phy_comm):
    - 0: MM_CEN_A
    - 0xFF: MM_INVALID (if transmission branch is not valid)

  Example:
    <code>
    phyBand = SRV_PCOUP_Get_Phy_Band(SRV_PLC_PCOUP_MAIN_BRANCH);

    if (phyBand == MM_CEN_A)
    {

    }
    </code>

  Remarks:
    None.
  ***************************************************************************/

uint8_t SRV_PCOUP_Get_Phy_Band(SRV_PLC_PCOUP_BRANCH branch);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //SRV_PCOUP_H
