![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# Microchip MPLAB® Harmony 3 Release Notes

## Harmony 3 Smart Energy Meters And More application examples v1.0.0

### Development kit and demo application support

The following development kits are used on provided Meters And More Demo Applications:

- [PIC32CXMTG-EK Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV11K09A)
- [PIC32CXMTSH-DB Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV84M21A)
- [PL460 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV13L63A)

### New Features

The following table provides a list of available applications, supported platforms and a brief description of functionalities:

| Application | Platform | Description |
| ----------- | -------- | ----------- |
| PHY Tx Test Console | [PIC32CX-MTG, PIC32CX-MTSH] + PL460-EK | PLC PHY demo application to manage PLC transmissions via serial console, including Sniffer functionality to monitor data traffic on the Network |
| Modem App | PIC32CX-MTG + PL460-EK | Meters And More Node serialized on top of AL and DLL Layers, which can be initialized as Master or Slave Node, and further controlled via serialization |
| Meter App | PIC32CX-MTG + PL460-EK | Meters And More Slave Node provided as example of an implementation which would be part of a Meter |
| DCU App | PIC32CX-MTG + PL460-EK | Meters And More Master Node provided as example of an implementation which would be part of a Data Concentrator Unit (DCU) |

### Known Issues

- None.

### Development Tools

- [MPLAB® X IDE v6.20](https://www.microchip.com/mplab/mplab-x-ide)
- [MPLAB® XC32 C/C++ Compiler v4.45](https://www.microchip.com/mplab/compilers)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator 5.5.1 or higher

In order to regenerate source code for any of the applications, you will also need to use the following versions of the dependent modules (see smartenergy_metersandmore_apps/package.yml):

- Harmony core repository, v3.14.1
- Harmony csp repository, v3.20.0
- Harmony shd repository, v1.0.0-E1
- Harmony smartenergy repository, v1.3.0
- Harmony smartenergy_metersandmore repository, v1.0.0
- Harmony crypto_v4 repository, v4.0.0-E3
- Harmony wolfssl repository, v5.7.0
- Harmony gfx repository, v3.15.3
- CMSIS_5 v5.9.0 (https://github.com/ARM-software/CMSIS_5/tree/5.9.0)
