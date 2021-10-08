## I/O Overview
There are many peripherals and devices connected to the mainboard. Every component that interfaces with the mainboard's microcontroller is listed here:
- **CAN-buses** (CAN): A CAN-bus to communicate with the car and an internal isolated bus for handling the cellboards.
- **Si8900 isolated ADC** (UART): Used to measure pack and bus voltages, and is also used to measure the pack current using a shunt resistor.
- **DAHB S/160 Hall-effect sensor** (ADC): Used to measure pack current. Less precise than the shunt, included for historic reasons. The unit contains two sensors with different gains to have a broader current range.
- **AIR and pre-charge controls**: Three output pins control the actuation of the AIRs (if the shutdown circuit is not latched in the off state) and the pre-charge circuit.
- **Circuit feedbacks** (MUX/ADC): 16 multiplexed analog signals coming from the mainboard's circuit. Used to diagnose the circuit and to verify its correct state compared to the BMS state. Some of those feedbacks are not multiplexed to permit the generation of interrupts from them.
- **IMD Status** (PWM): The IMD has an output PWM signal that reports its internal state.
- **M95256 EEPROM** (SPI): Used to store runtime variables and data, such as state of charge information, balancing threshold and more.
- **SD-card** (SPI): An SD-card is included on the mainboard to be used as storage for extended logs.
- **User console** (UART): A serial interface is reserved as an user interface to read/write data and execute commands. The [cli](https://github.com/eagletrt/micro-libs/tree/master/cli) library is used as an interface.
- **GPIOs**: 9 GPIO pins for external stuff.

![Mainboard's I/O configuration](/pics/mainboard_cubemx.png)