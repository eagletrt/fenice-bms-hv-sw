# Timebase
It sets a timer to tick every 10ms, then sets the respective flag whenever a interval is matched. By calling it's routine function from the main loop is possible to execute the needed code at fixed intervals.
Specifically is used to read voltages, current and temperatures, to send periodic can messages and to control fans.