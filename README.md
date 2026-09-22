# Arduino Nano Dual-Channel Digital Wax Carver (AVR Version)

An advanced, object-oriented C++ firmware written for the **ATmega328P (Arduino Nano)** architecture to control a professional dual-channel digital dental wax carver / electric spatula station equipped with emergency hardware interrupts and a TM1638 display controller.

## Features
- **Object-Oriented Architecture:** Fully modular C++ codebase structure cleanly separating tasks into decoupled handlers (`ButtonHandler`, `MenuHandler`, `TM1638_Custom` driver classes).
- **Hardware-Level Emergency Protection:** Leverages native AVR external interrupts (`INT0` / `INT1` on pins `D2/D3`) running on high-priority `FALLING` triggers to instantly kill low-side power gates and flag an execution lock (`Err`) in less than a microsecond if an over-current or cooling fault occurs.
- **Advanced Button Debouncing & Auto-Repeat:** Implements state-machine tracking to handle concurrent actions, dynamic 50ms debouncing, and fluid continuous values acceleration (auto-scroll) when holding down arrow keys.
- **Visual Menu Interface:** Built-in settings matrix allowing operators to tweak operational thermal parameters, adjust buzzer audio, toggle individual channel outputs, and configure timed turbo parameters.
- **Non-Volatile EEPROM Persistence:** Automatically flushes updated target point indices directly into the ATmega328P's internal non-volatile EEPROM memory arrays to sustain parameters across power cycles.

## Hardware Pinout Configuration Blueprint (Arduino Nano)
- `D2 / D3` -> Hardware Emergency Interrupt Fault Inputs (`PROT_CH1` / `PROT_CH2`)
- `D5 / D6` -> Native AVR 8-bit Hardware PWM Channels (`HEAT_CH1` / `HEAT_CH2`)
- `D4 / D7 / D8` -> TM1638 Panel Serial Interface (`STB` / `CLK` / `DIO`)

## UI Menu Structures
Hold the **TURBO** button for 5 seconds to enter the digital configuration matrix:
- `t1_` / `t1~` : Set Minimum / Maximum Allowed Temperature for Channel 1.
- `t2_` / `t2~` : Set Minimum / Maximum Allowed Temperature for Channel 2.
- `trbo` (Time) : Configure Turbo Burst duration limit (up to 999 seconds with 's' suffix formatting).
- `trbO` (Temp) : Set Turbo Target Temperature.
- `Snd ` : Toggle active audio feedback states.
- `Out1` / `Out2` : Toggle structural channel operational loops.

## Usage
1. Open the project loop layout within the Arduino IDE.
2. Ensure you select **Arduino Nano (ATmega328P)** inside the board manager panel.
3. Flash the bundle directly over the standard USB-UART bootloader interface interface.

## Original Credits
Developed by **DenGame** (AVR Release). Compiled using high-performance embedded structural models to protect microcontroller memory layouts.
