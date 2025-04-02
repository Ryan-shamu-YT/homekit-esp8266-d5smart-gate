# HomeKit Gate Controller for Centurion D5-Evo Gate Motor

**HomeKit gate controller for Centurion (Centsys) D5 Evo gate motor**

This project implements a HomeKit-compatible gate controller using an ESP8266 microcontroller and the Arduino-HomeKit-ESP8266 library, specifically designed for Centurion (Centsys) D5-Evo gate motors. It enables remote control and status monitoring of the gate with Apple HomeKit.

## Features
- ✅ **HomeKit Integration:** Seamless control via the Apple Home app using the HomeSpan library.
- ✅ **Gate Control:** Open and close the gate with a relay trigger.
- ✅ **Status Detection:** Real-time detection of gate states (open, closed, opening, closing).
- ✅ **Gate Status** Detects when gate opens/closes from a different source (e.g. remote control, intercom, keypad).

## How It Works
The system uses an ESP8266 microcontroller to manage a relay and monitor the gate's operational state:
- **Relay Control:** The relay pin (GPIO 14) triggers the gate motor.
- **Status Monitoring:** The status pin (A0) monitors the gate's operation based on voltage sent by the gate controller.
- **Flashing Patterns:** The gate signals its state using voltage levels, where the voltage of the IO/4 indicates whether the gate is opening or closing.
- **Voltage Conversion:** A VOLTAGE DIVIDING circuit is used to shift the gate's 5V status signal to a 1V input suitable for the ESP8266 A0 pin.

The `main.ino` class implements:
- `update()` – Triggered by HomeKit to open/close the gate.
- `loop()` – Continuously monitors the gate's state based on the input pin.

## Hardware Requirements
- ESP8266 microcontroller
- Relay module (for gate control)
- Buck Converter (Optional if you want to use another source of power)
- 10K Resistor and 40k Resistor (5V to 1V signal conversion on A0 pin)

## Wiring

### Gate Motor, LLC, Relay Module, Buck Converter, and ESP8266 Connections

#### 1. **Gate Motor to Buck Converter:**
- **COM** → **Buck Converter IN-** (Use ground from gate motor for isolation).
- **+12V Out** → **Buck Converter IN+** (12V output for buck converter input).
  
#### 2. **Gate Motor to Relay Module:**
- **COM (Gate Motor)** → **Relay COM** (Trigger input for gate motor, activated by relay).
- **I/O1** → **Relay NO** (Trigger input for gate motor, connected to Normally Open terminal of relay).

#### 3. **ESP A0 Connections:**
- **A0** → **10K Resistor** → **I/04** (Status signal from gate motor to Resistor).
- **A0** → **40k Resistor** → **COM (GATE)** (Negative terminal of gate to resistor).

#### 4. **Relay Module to ESP8266:**
- **IN** → **ESP8266 GPIO 14** (Control signal from GPIO 14, triggering relay).
- **NO** → **Gate Motor I/O1** (Normally Open terminal to trigger gate motor).
- **COM** → **Gate COM** (Connected to the gate COM for isolation).
- **VCC** → **Buck Converter OUT+** (Power for relay module).
- **GND** → **BUCK CONVERTER OUT -** (Shared ground for all components).

#### 5. **Buck Converter to Power Sources:**
- **VIN (+)** → **Gate Motor +12V Out** (12V input from gate motor to buck converter).
- **VOUT (-)** → **Relay GND** (5V ground output).
- **VOUT (+)** → **ESP8266 Power**, **LLC HV**, **Relay VCC** (5V output for powering ESP8266 and other components).
- **VIN (-)** → **COM** (Shared ground).

#### 7. **ESP8266  and Relay Module:**
- **GND** → **COM** (Shared ground).
- **3.3V** → **Relay VCC** (Power for relay).
- **GPIO 14** → **Relay IN** (Control signal to relay).
                                                 |

**Notes:**

* **Common Ground:** All components share a common ground connection possibly in a .
* **Buck Converter:** The buck converter efficiently steps down the 12V from the gate motor to 5V (and potentially 3.3V) for the other components.
* **Relay:** The relay acts as the switch to trigger the gate motor's movement.


![428311938-f44cfe6e-a5c6-495d-8480-1c9bc4ff0227](https://github.com/user-attachments/assets/ce4036c2-da26-44a4-bb67-4f9955ee7813)

Only 5 connections need to be made to the D5 SMART Gate Motor:
1. Trig(I/O1): this needs to be connected to ground for the gate to activate. I have connected this to a relay controlled by the ESP8266
2. Status(I/O4): this needs to be converted to 1V. Connect to an ADC pin (A0) of the ESP8266 with a 
3. Com (COM, ground, 0V): this is important for two reason: (1) status decoding and (2) triggering (activating) the gate.
4. 12V OUT (12V OUT, 12V): this is important for powering
(1), **the gate must share a ground with the ESP8266**. 


## Software Requirements
- [Arduino-HomeKit-ESP8266](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266)
- Arduino IDE (for programming the ESP8266)

## Configuration
- **Wi-Fi:** Set credentials in `wifi_info.h`.
- **HomeKit Pin:** Default pairing pin is `11111111 (8 ones)`.
- **Pins:** Adjust `statusPin` and `relayPin` if necessary.
  
## Installation
1. Install the HomeSpan library in Arduino IDE.
2. Download and open both `main.ino`, `my_accessory.c ` and `wifi_info.h` in an IDE.
3. Configure Wi-Fi credentials in `wifi_info.h`, HomeKit pairing code in `my_accessory.c`.
4. Upload the files to your ESP8266.
5. Pair the device with HomeKit using the code.

## Future Improvements
- **Obstruction Detection:** Currently not implemented.

## License
This project is open-source under the MIT License.

## Contributing
Contributions are welcome! Feel free to submit pull requests or open issues for improvements and bug fixes.
