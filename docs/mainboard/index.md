# Mainboard

**The Mainboard is the central control unit of the BMS**.\
It contains a microcontroller that handles two CAN-bus lines for internal and external communications, peripherals such as insulated *ADCs*, *EEPROMs*, serial ports, an SD-card and more.\
The mainboard is **responsible for the actuation of the AIRs and contains the shutdown and pre-charge circuits**.\
It also communicates voltages, temperatures, currents, battery status, warnings and errors to the rest of the car via CAN-bus. An integrated serial command-line interface and internal logging are included to help with troubleshooting.

The logic of the mainboard is handled by a couple of FSM *<small>(Finite States Machines)</small>*. The [FSM](https://github.com/eagletrt/micro-libs/tree/master/fsm) library is used to manage every state machine of the *BMS*.

---

## Subsystems

There are several subsystems to the mainboard's firmware:

- **[bal](subsystems/bal)**: contains functions of the cell balancing algorithm.
- **[bal_fsm](subsystems/bal)**: contains the balancing algorithm's FSM.
- **[bms_fsm](subsystems/bms_fsm)**: the BMS' FSM handles the main state machine of the board and it's responsible for the management of the tractive system's activation and deactivation.
- **[cli_bms](subsystems/cli_bms)**: all the CLI commands are defined here.
- **[config](subsystems/config)**: generic interface that handles all sorts of configurations.
- **[current](subsystems/current)**: current measurement functions.
- **[energy](subsystems/energy)**: energy measurement and State-of-Charge estimation logic.
- **[feedback](subsystems/feedback)**: mainboard's feedbacks handler functions and variables.
- **[pack](subsystems/pack)**: battery pack hardware control's interface.
- **[soc](subsystems/soc)**: State-of-Charge estimation functions *(at the moment it doesn't work)*.
- **[timebase](subsystems/timebase)**: controls all repeating actions of the firmware.
