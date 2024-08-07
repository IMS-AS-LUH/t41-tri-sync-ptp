# Tri-Level Sync Generation based on a PTP-synchronized clock with the Teensy 4.1

This [repository](https://github.com/IMS-AS-LUH/t41-tri-sync-ptp) provides the source code for our (yet unpublished) research paper "PTP-Synchronized Tri-Level Sync Generation for Networked Multi-Sensor Systems". This Arduino compatible library has been created to provide a Tri-Level Sync signal generation based on a PTP-Synchronized clock for the Teensy 4.1 microcontroller platform.

This library was created by the Architectures and Systems Group of the Institute of Microelectronic Systems ([IMS/AS](https://www.ims.uni-hannover.de/de/institut/architekturen-und-systeme/)) at the [Leibniz University](https://www.uni-hannover.de) in Germany.

## Getting Started

### Prerequisites

_(based on the [ptp library](https://github.com/IMS-AS-LUH/t41-ptp))_

- Teensy 4.1 with Ethernet adapter
- Working Teensy Arduino environment
- PTP-branch of QNEthernet Library with our patches: https://github.com/HedgeHawk/QNEthernet/tree/ieee1588-2-fix

### Installation

- Clone the library folder to your Arduino-libraries path
- Restart Arduino IDE

### Examples

Additionally to the PTP Slave and PTP Master example of the base repository, an extra example is provided for the generation of the Tri-Level Sync signal generation.

### Board Setup

The signal generation requires an extra electrical network connected to the microcontroller. The schematic can be seen here:
![electrical network](doc/trisyncnetwork.png)
The lower and higher pin for the example are pin 10 and 12.

## Citation

If you use our Tri-Level Sync library (or parts of it) in scientific work, please cite our paper (not published yet):

```
@unpublished{riggers2024tri,
title={Sub-Microsecond Time Synchronization for Network-Connected Microcontrollers},
author={Christoph Riggers and Jens Schleusner and Oliver Renke and Holger Blume},
booktitle="2024 IEEE 30th International Conference on Embedded and Real-Time Computing Systems and Applications (RTCSA)",
month=aug,
year=2024,
}
```

The correct precise citation will be updated as soon as possible.

This work is based on the [ptp library](https://github.com/IMS-AS-LUH/t41-ptp). If you are using the
PTP functionality of this repository, cite them accordingly.

## License

Distributed under the MIT license. Please see LICENSE for more information.
