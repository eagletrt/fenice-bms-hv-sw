# OpenBlt

Both cellboards' and mainboard's bootloaders are derived from openblt sources -> [OpenBlt Github](https://github.com/feaser/openblt). Actually Core sources and Target specific sources have copied in the BLT folder. The whole bootloader configuration happens in the blt_conf.h header file, exception made for the flashLayout[] variable found in flash.c, where the flash layout is specified.

## Build and flash the bootloader

Building and flashing theese bootloaders is straightforward. Being STM32CubeMX projects, you just need the STM32-for-VSCode extention, build and flash. Done.

## Build the firmware

There are 2 main modification to be done on the firmware side in order to operate with the openblt bootloader.

- Modify the linker file (.ld file in the root directory) and shift the program in the flash. This can be done by changing the FLASH (rx) (line 63) accordingly with what specified in the previously mentioned flash.c file (eg. ```ORIGIN = 0x8004000```). Take care of reducing the flash size too (eg. ```LENGHT = 496K```).
- Convert the .bin build file to .srec format. This can be achieved with this tool: [arkku/srec](https://github.com/arkku/srec).  
Example: ```bin2srec -a 0x8004000 (or anithing specified by the linker file) -i %.bin -o %.srec```

## Flash the firmware

Both telemetry and steering wheel raspberries are already configured in order to flash by can. You just need to copy the .srec file in the ```/home/pi/can_flashing/fw``` folder.
Then you need to call the ```/home/script/can_flashing/scripts/flash_*``` script in order to flash the mainboard (```flash_mainboard```) or a specific cellboard (```flash_cellboard<n>```). You can also flash all the cellboard together with the ```flash_cellboard_all``` script.  
Those scripts just takes care of rebooting the target board with a specific can message and then calls the ```bootcommander``` tool.  
For how the scripts are written (demmerda e velocemente) the mainboard firmware must be called ```fenice-bms.srec``` and the cellboard one must be called ```cellboard.srec```.