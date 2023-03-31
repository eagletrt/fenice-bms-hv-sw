/**
 * @file		bootloader.h
 * @brief		functions and defines to jump to the bootloader
 *
 * @date		Mar 16, 2022
 * @author		Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#include "main.h"

#define BOOTLOADER_ADDR 0x8000000

void JumpToBlt();