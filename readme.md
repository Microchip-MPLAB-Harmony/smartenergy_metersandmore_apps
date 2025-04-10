![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# MPLAB® Harmony 3 Smart Energy Meters And More application examples

MPLAB® Harmony 3 is an extension of the MPLAB® ecosystem for creating embedded firmware solutions for Microchip 32-bit SAM and PIC® microcontroller and microprocessor devices. Refer to the following links for more information.

- [Microchip 32-bit MCUs](https://www.microchip.com/design-centers/32-bit)
- [Microchip 32-bit MPUs](https://www.microchip.com/design-centers/32-bit-mpus)
- [Microchip MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide)
- [Microchip MPLAB® Harmony](https://www.microchip.com/mplab/mplab-harmony)
- [Microchip MPLAB® Harmony Pages](https://microchip-mplab-harmony.github.io/)

This repository contains the MPLAB® Harmony 3 Smart Energy Meters And More application examples.

- [Release Notes](./release_notes.md)
- [MPLAB® Harmony License](Microchip_SLA001.md)

# Documentation

Click [here](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=MPLAB_Harmony_Smart_Energy_Meters_And_More_Applications&redirect=true) to view the online documentation of MPLAB® Harmony 3 Smart Energy Meters And More Stack.

To view the documentation offline, follow these steps:
 - Download the publication as a zip file from [here](https://onlinedocs.microchip.com/download/GUID-A95FCA59-E0E2-454E-9A79-5946EDE2DFB0?type=webhelp).
 - Extract the zip file into a folder.
 - Navigate to the folder and open **index.html** in a web browser of your choice.

# Contents Summary

| Folder | Description                                                      |
| ---    | ---                                                              |
| apps   | Contains Meters And More PHY and full-stack example applications |

# Code Examples

The following applications are provided to demonstrate the typical use cases of G3 at both PHY and full stack levels.

| Name               | Description |
| ----               | ----------- |
| [PHY Tx Test Console](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_MM_PHY_TX_Test_console&redirect=true) | Example that demonstrates the complete performance of the Microchip Meters And More PLC PHY Layer, avoiding timing limitations in the PC host. That way, users can perform more specific PHY tests (e.g., short time interval between consecutive frames). It also includes a Sniffer functionality to monitor PLC traffic in the Network. |
| [Meters And More Modem HI](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_MM_Modem_App&redirect=true) | Modem implementation of a Meters And More Node, implementing the standard Host Interface as defined in Certification documents, as the Serial interface with Host. |
| [Meters And More Modem USI](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_MM_Modem_App_USI&redirect=true) | Modem implementation of a Meters And More Node, serialized on top of AL Layer, which can be initialized as Master or Slave Node, and further controlled via serialization by means of Smart Energy USI (Unified Serial Interface) Service. |
| [Meters And More Meter](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_MM_Meter_App&redirect=true) | Example implementation of the Meter (Slave) side of a Meters And More Node. |
| [Meters And More DCU](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_MM_DCU_App&redirect=true) | Example implementation of the Data Concentrator (Master) side of a Meters And More Node. |

____

[![License](https://img.shields.io/badge/license-Harmony%20license-orange.svg)](https://github.com/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps/blob/master/Microchip_SLA001.md)
[![Commit activity](https://img.shields.io/github/commit-activity/y/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps.svg)](https://github.com/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps/graphs/commit-activity)
[![Contributors](https://img.shields.io/github/contributors-anon/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps.svg)]()

____

[![Developer Help](https://img.shields.io/badge/Youtube-Developer%20Help-red.svg)](https://www.youtube.com/MicrochipDeveloperHelp)
[![Developer Help](https://img.shields.io/badge/XWiki-Developer%20Help-torquiose.svg)](https://developerhelp.microchip.com/xwiki/bin/view/software-tools/harmony/)
[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/user/MicrochipTechnology)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/microchip-technology)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/microchiptechnology/)
[![Follow us on Twitter](https://img.shields.io/twitter/follow/MicrochipTech.svg?style=social)](https://twitter.com/MicrochipTech)

[![](https://img.shields.io/github/stars/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps.svg?style=social)]()
[![](https://img.shields.io/github/watchers/Microchip-MPLAB-Harmony/smartenergy_metersandmore_apps.svg?style=social)]()