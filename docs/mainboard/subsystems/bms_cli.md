# Command Line Interface

The command line interface of the Mainboard is based on the [cli](https://github.com/eagletrt/micro-libs/tree/master/cli) library. It serves as the main interface with the BMS. The CLI is clocked at 115200 baud and uses local echo

!!! warning
	The CLI runs on UART1, thus it is not compatible with STM32 Nucleo boards that normally use UART2 as the USB serial console.

As the name implies, the CLI is based around commands, in a style inspired by Unix consoles. Pressing `?` followed by `Enter` will give a list of legal commands.

## Commands
### `volt`
The `volt` command returns the main voltages of the battery pack.

!!! example
	```
	bus.......0.00 V
	internal..388.8 V
	average...3.60 V
	max.......3.70 V
	min.......3.50 V
	delta.....0.20 V
	```

#### Parameters
- `all`: prints all cell voltages (plus some usefull information about balancing).

### `temp`
Similarly to the `volt` command, `temp` will return the main temperatures of the pack.
#### Parameters
- `all`: Prints all battery temperatures.

### `status`
Returns a summary of the general status of the BMS.

### `errors`
This command will return a list containing each active errors and its details

### `ts`
Controls the Tractive System actuation.
#### Parameters
- `on`: generates a `TS_ON` event on the [BMS FSM](../bms_fsm).
- `off`: triggers `TS_OFF` event on the aforementioned FSM.

### `bal`
Suite of commands that handle the balancing process.
#### Parameters
- `on`: Triggers an event that enables the [BAL FSM](../bal_fsm).
- `off`: Triggers an event that disables balancing.
- `thr`: Returns the currently set threshold.
- `thr <millivolts>`: Sets the threshold to the value of `<millivolts>`.
    Please be gentle with it: it can break bad if you input strange stuff
- `test <board> <cell0 cell1 ... cellN>`: starts a balancing cycle in the `board` on the specified cells

### `soc`
Return state of charge information
#### Parameters
- `reset`: resets the 'last charge' energy meter.

### `current`
Shows current measurement for all the available sensors.
#### Parameters
- `zero`: zeroes the Hall-effect sensors.

### `dmesg`
Toggles the debug output. This can be quite verbose if enabled.

### `reset`
Resets the microcontroller. Analogue to pressing the reset button on board.

### `imd`
Show imd details

### `feedbacks`
Show feedbacks status

### `watch`
Repeatedly run a command
#### Parameters
- `<interval ms> <cmd>`

### `cell_distr`
Lets you change the cellboard arrangment into the pork
#### Parameters
- ``: Show current cellboard distribution
- `<cell0> ... <cellN>`: Update cellboard distribution

### `fans`
Control the pork fans
#### Parameters
- `off`: shut down fans
- `<0-100>`: sets fans power

### `pack`
Control low level ts hardware
!!! warning
	This can lead to dangerous behaviours when the tractive system is connected, be carefull
#### Parameters
- `airn <on/off>`: controls the air negative
- `airp <on/off>`: controls the air positive
- `precharge <on/off>`: controls the precharge relay

### Easter eggs?
Of course
