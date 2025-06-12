# VCU25v1 - Formula SAE Vehicle Control Unit

## Overview

This project implements a Vehicle Control Unit (VCU) for Formula SAE electric vehicles using the **STM32F756ZG Discovery Board**. The VCU manages accelerator pedal inputs, performs safety checks, and communicates torque commands to the motor inverter via CAN bus.

## Target Hardware

- **STM32F756ZG Discovery Board**
- **On-board LEDs**: LD1 (Green), LD2 (Blue), LD3 (Red)
- **CAN Transceivers**: Dual CAN buses for motor control and telemetry
- **ADC Inputs**: Multiple channels for pedal position sensing

## System Architecture

The VCU uses **FreeRTOS** with four main tasks running concurrently:

1. **InverterProcess Task** - Main VCU control loop that sends CAN commands to the motor inverter
2. **Plausibility Task** - Safety checks and torque mapping from pedal position
3. **AppsVerify Task** - Accelerator Pedal Position Sensor (APPS) data processing and fault detection
4. **AppsCalibrate Task** - One-time calibration of APPS sensors (runs once at startup)

## Key Features

### APPS (Accelerator Pedal Position Sensor) System
- **Dual redundant sensors** for safety compliance (typically potentiometers)
- **Automatic calibration** process on startup
- **Real-time sensor agreement checking** with configurable tolerance
- **Fault detection** and safe shutdown capability

### Safety Systems
- **Plausibility checks** between accelerator and brake pedal positions
- **Sensor fault detection** with automatic inverter disable
- **10% sensor agreement tolerance** with 100ms fault timeout
- **Safe torque mapping** with configurable lookup tables

### Torque Control
- **Linear interpolation** between predefined torque map points
- **Maximum torque limit**: 75 Nm (configurable in plausibility.c)
- **CAN-based communication** to motor inverter
- **Real-time torque command updates** at 1kHz

## User LED Functions

The three on-board LEDs provide real-time system status:

### LD1 (Green) - APPS Sensor Fault
- **OFF**: APPS sensors operating normally with good agreement
- **FLASHING**: APPS sensor disagreement detected (>10% difference for >100ms)
- Indicates potential wiring issues or sensor failure

### LD2 (Blue) - APPS Maximum Calibration
- **ON**: System is in maximum calibration mode (first 3 seconds after startup)
- **OFF**: Normal operation or calibration complete
- During calibration: **Press accelerator pedal to maximum position**

### LD3 (Red) - APPS Minimum Calibration  
- **ON**: System is in minimum calibration mode (seconds 3-6 after startup)
- **OFF**: Normal operation or calibration complete
- During calibration: **Release accelerator pedal completely**

## How to Use

### Initial Setup and Calibration

1. **Connect the STM32F756ZG Discovery Board** to your development environment
2. **Flash the firmware** using STM32CubeIDE or your preferred toolchain
3. **Power on the board** (see Power Configuration section below)
4. **Press the RESET button** to begin the calibration sequence

### Calibration Process (First 6 seconds after reset)

1. **Seconds 0-3 (LD2 ON)**: Press and hold the accelerator pedal to **maximum position**
2. **Seconds 3-6 (LD3 ON)**: Release the accelerator pedal to **minimum position** 
3. **After 6 seconds**: Normal operation begins, all LEDs should be OFF during normal operation

### Normal Operation

- The VCU will continuously read APPS sensors and send torque commands via CAN
- Monitor LED status for any fault conditions
- LD1 flashing indicates sensor issues that require attention

## Power Configuration with Vin Power

### Power Jumper Settings

When using external Vin power supply (recommended for vehicle integration):

1. **Locate the power jumper JP1** on the STM32F756ZG Discovery Board
2. **Remove the jumper from the default USB power position** (usually pins 1-2)
3. **Place the jumper on pins 2-3** to enable Vin power input
4. **Connect your external power supply** (7-12V DC) to the Vin terminal

### ⚠️ Important Power-Up Procedure

**CRITICAL**: After connecting Vin power, you **MUST press the RESET button** on the Discovery Board to begin code execution. The board will not automatically start running your firmware when switching from USB to Vin power.

**Power-Up Steps**:
1. Set power jumper to Vin position (pins 2-3)
2. Connect external power supply to Vin terminal
3. **Press the black RESET button** on the board
4. Observe LED calibration sequence starting immediately

### Power Supply Requirements
- **Voltage**: 7-12V DC
- **Current**: Minimum 500mA (1A recommended)
- **Connector**: 2.1mm barrel jack or wire to Vin terminal
- **Polarity**: Center positive

## CAN Bus Configuration

The VCU uses dual CAN buses:
- **CAN1**: High-priority motor inverter commands (500 kbps)
- **CAN2**: Telemetry and dashboard communication (250 kbps)

Ensure proper CAN termination (120Ω resistors) at both ends of each bus.

## ADC Channel Mapping

- **ADC3 Channel 5**: APPS Sensor 1
- **ADC3 Channel 7**: APPS Sensor 2  
- **ADC1**: Brake position sensors (if implemented)

## Troubleshooting

### LD1 Flashing (APPS Fault)
- Check APPS sensor wiring and connections
- Verify sensor power supply (typically 5V)
- Ensure both sensors are properly calibrated
- Check for loose connections or damaged potentiometers

### No Response After Power-On
- Verify power jumper position matches power source
- **Press RESET button** after connecting Vin power
- Check power supply voltage (7-12V DC)
- Verify firmware flashing was successful

### Calibration Issues
- Ensure accelerator pedal moves through full range
- Check that sensors provide different readings at min/max positions
- Re-flash firmware if calibration data appears corrupted

## Development Environment

- **IDE**: STM32CubeIDE (recommended) or Keil/IAR
- **HAL Version**: STM32F7 HAL Driver
- **RTOS**: FreeRTOS v10.x
- **Debug Interface**: ST-LINK v2 (built-in to Discovery Board)

## Safety Considerations

This VCU is designed for Formula SAE competition use and includes multiple safety systems. However, always verify proper operation before vehicle integration:

- Test all sensors and calibration procedures
- Verify CAN communication with motor inverter
- Confirm emergency shutdown capabilities
- Follow Formula SAE electrical regulations

## License

This project follows STMicroelectronics software license terms. See LICENSE file for details.