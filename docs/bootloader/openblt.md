# OpenBlt

---

## Table of Contents

- [Intro](#intro)
- [Building and flashing the bootloader](#building-and-flashing-the-bootloader)
- [Building the firmware](#building-the-firmware)
- [Flashing the firmware](#flashing-the-firmware)
---

# Intro

Both cellboards' and mainboard's bootloaders are derived from [OpenBlt sources](https://github.com/feaser/openblt).\
Actually *Core sources* and *Target specific sources* have been copied in the BLT folder. The whole bootloader configuration happens in the `blt_conf.h` header file, exception made for the `flashLayout[]` variable found in `flash.c`, where the flash layout is specified.

---

## Building and flashing the bootloader

Building and flashing theese bootloaders is straightforward.\
Being `STM32CubeMX` projects, you just need the `STM32-for-VSCode` extension, **build and flash. Done.**

---

## Building the firmware

There are **2 main modifications** that you need to do on the firmware side, in order to operate with the `openblt bootloader`.

1. **Modify the linker file** (`.ld` file in the root directory) and **shift the program** in the flash.\
This can be done by changing the `FLASH (rx) (line 63)` according to what has been specified in the previously mentioned `flash.c` file (eg. `ORIGIN = 0x8004000`). **Remember to reduce the flash size too** (eg. `LENGHT = 496K`).

2. **Convert the `.bin` build file to `.srec` format**.\
This can be achieved with this tool: [arkku/srec](https://github.com/arkku/srec).  
*Example:*
    ```c
    bin2srec -a 0x8004000 (or anithing specified by the linker file) -i %.bin -o %.srec
    ```

---

## Flashing the firmware

Both telemetry's and steering wheel's Raspberries have been configured in order to **flash by CAN**.

All you need to do is follow these steps:

1. Copy the `.srec` file in the `/home/pi/can_flashing/fw` folder

2. Call the `/home/script/can_flashing/scripts/flash_*` script, to flash the mainboard (`flash_mainboard`) or a specific cellboard (`flash_cellboard<n>`).
    > You can also flash all the cellboards together with the `flash_cellboard_all` script.  

> ***NB:***
> Those scripts just take care of rebooting the target board with a specific CAN message and then calls the `bootcommander` tool.  
> Because of the way the scripts have been written *<sup>(demmerda e velocemente)</sup>* the mainboard's firmware must be called `fenice-bms.srec` and the cellboard's one must be called `cellboard.srec`.