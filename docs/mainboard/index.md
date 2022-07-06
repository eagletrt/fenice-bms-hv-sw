# Mainboard

The Mainboard is the central control unit of the BMS. It contains a microcontroller that handles two CAN-bus lines for internal and external communications, peripherals such as insulated ADCs, EEPROMs, serial ports, an SD-card and more. The mainboard is responsible for the actuation of the AIRs and contains the shutdown and pre-charge circuits. It also communicates voltages, temperatures, currents, battery status, warnings and errors to the rest of the car via CAN-bus. An integrated serial command-line interface and internal logging are included to help with troubleshooting.

The logic of the mainboard is handled by a couple of finite state machines (FSM). The [fsm](https://github.com/eagletrt/micro-libs/tree/master/fsm) library is used to manage every state machine of the BMS.

## Subsystems
There are several subsystems to the Mainboard's firmware:

- **[bal](subsystems/bal)**: The bal subsystem contains functions of the cell balancing algorithm
- **[bal_fsm](subsystems/bal)**: The FSM of the balancing algorithm is contained here
- **[bms_fsm](subsystems/bms_fsm)**: The BMS fsm handles the main state machine of the board. It is responsible for the management of the tractive system activation and deactivation.
- **[cli_bms](subsystems/cli_bms)**: All the CLI commands are defined here.
- **[config](subsystems/config)**: Generic interface to handle all sorts of configurations
- **[current](subsystems/current)**: Current measurement functions
- **[energy](subsystems/energy)**: Energy measurement and State-of-Charge estimation logic.
- **[feedback](subsystems/feedback)**: Mainboard's feedbacks handler functions and variables
- **[pack](subsystems/pack)**: Interface for the battery pack hardware control.
- **[soc](subsystems/soc)**: State of charge estimation functions (not working).
- **[timebase](subsystems/timebase)**: Controls all repeating actions of the firmware.
