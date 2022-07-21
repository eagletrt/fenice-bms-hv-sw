/************************************************************************************//**
* \file         Source/ARMCM4_STM32L4/flash.c
* \brief        Bootloader flash driver source file.
* \ingroup      Target_ARMCM4_STM32L4
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2018  by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* This file is part of OpenBLT. OpenBLT is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* OpenBLT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License along with OpenBLT. It
* should be located in ".\Doc\license.html". If not, contact Feaser to obtain a copy.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "stm32l4xx.h"                           /* STM32 CPU and HAL header           */



/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Value for an invalid flash address. */
#define FLASH_INVALID_ADDRESS           (0xffffffff)
/** \brief Standard size of a flash block for writing. */
#define FLASH_WRITE_BLOCK_SIZE          (512)
/** \brief Standard size of a flash sector for erasing. */
#define FLASH_ERASE_SECTOR_SIZE         (2048)
/** \brief Total numbers of segments in array flashLayout[]. */
#define FLASH_TOTAL_SEGMENTS            (sizeof(flashLayout)/sizeof(flashLayout[0]))
/** \brief Index of the last segment in array flashLayout[]. */
#define FLASH_LAST_SEGMENT_IDX          (FLASH_TOTAL_SEGMENTS-1)
/** \brief Start address of the bootloader programmable flash. */
#define FLASH_START_ADDRESS             (flashLayout[0].sector_start)
/** \brief End address of the bootloader programmable flash. */
#define FLASH_END_ADDRESS               (flashLayout[FLASH_LAST_SEGMENT_IDX].sector_start + \
                                         flashLayout[FLASH_LAST_SEGMENT_IDX].sector_size - 1)
/** \brief Offset into the user program's vector table where the checksum is located.
 *         For this target it is set to the end of the vector table. Note that the
 *         value can be overriden in blt_conf.h, because the size of the vector table
 *         could vary. When changing this value, don't forget to update the location
 *         of the checksum in the user program accordingly. Otherwise the checksum
 *         verification will always fail.
 */
#ifndef BOOT_FLASH_VECTOR_TABLE_CS_OFFSET
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET    (0x188)
#endif


/****************************************************************************************
* Plausibility checks
****************************************************************************************/
#if (BOOT_FLASH_VECTOR_TABLE_CS_OFFSET >= FLASH_WRITE_BLOCK_SIZE)
#error "BOOT_FLASH_VECTOR_TABLE_CS_OFFSET is set too high. It must be located in the first writable block."
#endif

#ifndef BOOT_FLASH_CUSTOM_LAYOUT_ENABLE
#define BOOT_FLASH_CUSTOM_LAYOUT_ENABLE (0u)
#endif


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Flash sector descriptor type. */
typedef struct
{
  blt_addr   sector_start;                       /**< sector start address             */
  blt_int32u sector_size;                        /**< sector size in bytes             */
} tFlashSector;

/** \brief    Structure type for grouping flash block information.
 *  \details  Programming is done per block of max FLASH_WRITE_BLOCK_SIZE. for this a
 *            flash block manager is implemented in this driver. this flash block manager
 *            depends on this flash block info structure. It holds the base address of
 *            the flash block and the data that should be programmed into the flash
 *            block. The .base_addr must be a multiple of FLASH_WRITE_BLOCK_SIZE.
 */
typedef struct
{
  blt_addr  base_addr;
  blt_int8u data[FLASH_WRITE_BLOCK_SIZE];
} tFlashBlockInfo;


/****************************************************************************************
* Hook functions
****************************************************************************************/
#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
extern blt_bool FlashCryptoDecryptDataHook(blt_int8u * data, blt_int32u size);
#endif


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static blt_bool   FlashInitBlock(tFlashBlockInfo *block, blt_addr address);
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr);
static blt_bool   FlashAddToBlock(tFlashBlockInfo *block, blt_addr address,
                                  blt_int8u *data, blt_int32u len);
static blt_bool   FlashWriteBlock(tFlashBlockInfo *block);
static blt_int32u FlashGetPage(blt_addr address);
static blt_int32u FlashGetBank(blt_addr address);


/****************************************************************************************
* Local constant declarations
****************************************************************************************/
/** \brief   If desired, it is possible to set BOOT_FLASH_CUSTOM_LAYOUT_ENABLE to > 0
 *           in blt_conf.h and then implement your own version of the flashLayout[] table
 *           in a source-file with the name flash_layout.c. This way you customize the
 *           flash memory size reserved for the bootloader, without having to modify
 *           the flashLayout[] table in this file directly. This file will then include
 *           flash_layout.c so there is no need to compile it additionally with your
 *           project.
 */
#if (BOOT_FLASH_CUSTOM_LAYOUT_ENABLE == 0)
/** \brief   Array wit the layout of the flash memory. Note that the current flash driver
 *           supports the STM32L4x1, STM32L4x5 and STM32L4x6 derivatives in the STM32L4
 *           family of microcontrollers.
 *  \details Also controls what part of the flash memory is reserved for the bootloader.
 *           If the bootloader size changes, the reserved sectors for the bootloader
 *           might need adjustment to make sure the bootloader doesn't get overwritten.
 */
static const tFlashSector flashLayout[] =
{
  /* space is reserved for a bootloader configuration with all supported communication
   * interfaces enabled. when for example only UART is needed, then the space required
   * for the bootloader can be made a lot smaller here.
   */
  /* { 0x08000000, 0x00800 },           flash sector  0 - 2kb (reserved for bootloader)*/
  /* { 0x08000800, 0x00800 },           flash sector  1 - 2kb (reserved for bootloader)*/
  /* { 0x08001000, 0x00800 },           flash sector  2 - 2kb (reserved for bootloader)*/
  /* { 0x08001800, 0x00800 },           flash sector  3 - 2kb (reserved for bootloader)*/
  /* { 0x08002000, 0x00800 },           flash sector  4 - 2kb (reserved for bootloader)*/
  /* { 0x08002800, 0x00800 },           flash sector  5 - 2kb                       */
  /* { 0x08003000, 0x00800 },           flash sector  6 - 2kb                       */
  /* { 0x08003800, 0x00800 },           flash sector  7 - 2kb                       */
  { 0x08004000, 0x00800 },              /* flash sector  8 - 2kb                       */
  { 0x08004800, 0x00800 },              /* flash sector  9 - 2kb                       */
  { 0x08005000, 0x00800 },              /* flash sector 10 - 2kb                       */
  { 0x08005800, 0x00800 },              /* flash sector 11 - 2kb                       */
  { 0x08006000, 0x00800 },              /* flash sector 12 - 2kb                       */
  { 0x08006800, 0x00800 },              /* flash sector 13 - 2kb                       */
  { 0x08007000, 0x00800 },              /* flash sector 14 - 2kb                       */
  { 0x08007800, 0x00800 },              /* flash sector 15 - 2kb                       */
  { 0x08008000, 0x00800 },              /* flash sector 16 - 2kb                       */
  { 0x08008800, 0x00800 },              /* flash sector 17 - 2kb                       */
  { 0x08009000, 0x00800 },              /* flash sector 18 - 2kb                       */
  { 0x08009800, 0x00800 },              /* flash sector 19 - 2kb                       */
  { 0x0800A000, 0x00800 },              /* flash sector 20 - 2kb                       */
  { 0x0800A800, 0x00800 },              /* flash sector 21 - 2kb                       */
  { 0x0800B000, 0x00800 },              /* flash sector 22 - 2kb                       */
  { 0x0800B800, 0x00800 },              /* flash sector 23 - 2kb                       */
  { 0x0800C000, 0x00800 },              /* flash sector 24 - 2kb                       */
  { 0x0800C800, 0x00800 },              /* flash sector 25 - 2kb                       */
  { 0x0800D000, 0x00800 },              /* flash sector 26 - 2kb                       */
  { 0x0800D800, 0x00800 },              /* flash sector 27 - 2kb                       */
  { 0x0800E000, 0x00800 },              /* flash sector 28 - 2kb                       */
  { 0x0800E800, 0x00800 },              /* flash sector 29 - 2kb                       */
  { 0x0800F000, 0x00800 },              /* flash sector 30 - 2kb                       */
  { 0x0800F800, 0x00800 },              /* flash sector 31 - 2kb                       */
  { 0x08010000, 0x10000 },              /* flash sectors 32 to 63 - 64kb               */
  { 0x08020000, 0x20000 },              /* flash sectors 64 to 127 - 128kb             */
#if (BOOT_NVM_SIZE_KB > 256)
  { 0x08040000, 0x40000 },              /* flash sectors 128 to 255 - 256kb            */
#endif
#if (BOOT_NVM_SIZE_KB > 512)
  { 0x08080000, 0x80000 }               /* flash sectors 256 to 511 - 512kb            */
#endif
#if (BOOT_NVM_SIZE_KB > 1024)
#error "BOOT_NVM_SIZE_KB > 1024 is currently not supported."
#endif
};
#else
#include "flash_layout.c"
#endif /* BOOT_FLASH_CUSTOM_LAYOUT_ENABLE == 0 */


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief   Local variable with information about the flash block that is currently
 *           being operated on.
 *  \details The smallest amount of flash that can be programmed is
 *           FLASH_WRITE_BLOCK_SIZE. A flash block manager is implemented in this driver
 *           and stores info in this variable. Whenever new data should be flashed, it
 *           is first added to a RAM buffer, which is part of this variable. Whenever
 *           the RAM buffer, which has the size of a flash block, is full or  data needs
 *           to be written to a different block, the contents of the RAM buffer are
 *           programmed to flash. The flash block manager requires some software
 *           overhead, yet results is faster flash programming because data is first
 *           harvested, ideally until there is enough to program an entire flash block,
 *           before the flash device is actually operated on.
 */
static tFlashBlockInfo blockInfo;

/** \brief   Local variable with information about the flash boot block.
 *  \details The first block of the user program holds the vector table, which on the
 *           STM32 is also the where the checksum is written to. Is it likely that
 *           the vector table is first flashed and then, at the end of the programming
 *           sequence, the checksum. This means that this flash block need to be written
 *           to twice. Normally this is not a problem with flash memory, as long as you
 *           write the same values to those bytes that are not supposed to be changed
 *           and the locations where you do write to are still in the erased 0xFF state.
 *           Unfortunately, writing twice to flash this way, does not work reliably on
 *           all micros. This is why we need to have an extra block, the bootblock,
 *           placed under the management of the block manager. This way is it possible
 *           to implement functionality so that the bootblock is only written to once
 *           at the end of the programming sequence.
 */
static tFlashBlockInfo bootBlockInfo;


/************************************************************************************//**
** \brief     Initializes the flash driver.
** \return    none.
**
****************************************************************************************/
void FlashInit(void)
{
  /* init the flash block info structs by setting the address to an invalid address */
  blockInfo.base_addr = FLASH_INVALID_ADDRESS;
  bootBlockInfo.base_addr = FLASH_INVALID_ADDRESS;
} /*** end of FlashInit ***/


/************************************************************************************//**
** \brief     Reinitializes the flash driver.
** \return    none.
**
****************************************************************************************/
void FlashReinit(void)
{
  /* init the flash block info structs by setting the address to an invalid address */
  blockInfo.base_addr = FLASH_INVALID_ADDRESS;
  bootBlockInfo.base_addr = FLASH_INVALID_ADDRESS;
} /*** end of FlashReinit ***/


/************************************************************************************//**
** \brief     Writes the data to flash through a flash block manager. Note that this
**            function also checks that no data is programmed outside the flash
**            memory region, so the bootloader can never be overwritten.
** \param     addr Start address.
** \param     len  Length in bytes.
** \param     data Pointer to the data buffer.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashWrite(blt_addr addr, blt_int32u len, blt_int8u *data)
{
  blt_addr base_addr;

  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    return BLT_FALSE;
  }

  /* make sure the addresses are within the flash device */
  if ((addr < FLASH_START_ADDRESS) || ((addr+len-1) > FLASH_END_ADDRESS))
  {
    return BLT_FALSE;
  }

  /* if this is the bootblock, then let the boot block manager handle it */
  base_addr = (addr/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;
  if (base_addr == flashLayout[0].sector_start)
  {
    /* let the boot block manager handle it */
    return FlashAddToBlock(&bootBlockInfo, addr, data, len);
  }
  /* let the block manager handle it */
  return FlashAddToBlock(&blockInfo, addr, data, len);
} /*** end of FlashWrite ***/


/************************************************************************************//**
** \brief     Erases the flash memory. Note that this function also checks that no
**            data is erased outside the flash memory region, so the bootloader can
**            never be erased. Note that the term sector used by this flash driver is
**            equivalent to the term page in the STM32L4x reference manual.
** \param     addr Start address.
** \param     len  Length in bytes.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashErase(blt_addr addr, blt_int32u len)
{
  blt_addr erase_base_addr;
  blt_addr erase_current_addr;
  blt_int32u total_erase_len;
  blt_int16u nr_of_erase_sectors;
  blt_bool result = BLT_TRUE;
  blt_int16u sector_cnt;
  blt_int32u dummy;
  FLASH_EraseInitTypeDef eraseInitStruct;
  
  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    return BLT_FALSE;
  }
  
  /* determine the base address for the erase operation, by aligning to
   * FLASH_ERASE_SECTOR_SIZE.
   */
  erase_base_addr = (addr/FLASH_ERASE_SECTOR_SIZE)*FLASH_ERASE_SECTOR_SIZE;

  /* make sure the addresses are within the flash device */
  if ((erase_base_addr < FLASH_START_ADDRESS) || ((addr+len-1) > FLASH_END_ADDRESS))
  {
    return BLT_FALSE;
  }

  /* determine number of bytes to erase from base address */
  total_erase_len = len + (addr - erase_base_addr);

  /* determine the number of sectors to erase */
  nr_of_erase_sectors = (blt_int16u)(total_erase_len / FLASH_ERASE_SECTOR_SIZE);
  if ((total_erase_len % FLASH_ERASE_SECTOR_SIZE) > 0)
  {
    nr_of_erase_sectors++;
  }
  
  /* unlock access to the flash device */
  HAL_FLASH_Unlock();

  /* clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  
  /* prepare erase init structure */
  eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  eraseInitStruct.NbPages = 1;

  /* erase all sectors one by one */
  for (sector_cnt=0; sector_cnt<nr_of_erase_sectors; sector_cnt++)
  {
    /* keep the watchdog happy */
    CopService();
    /* set the sector base address */
    erase_current_addr = erase_base_addr + (sector_cnt * FLASH_ERASE_SECTOR_SIZE);
    /* update erase init structure */
    eraseInitStruct.Page = FlashGetPage(erase_current_addr);
    eraseInitStruct.Banks = FlashGetBank(erase_current_addr);
    /* perform sector erase operation */
    if (HAL_FLASHEx_Erase(&eraseInitStruct, (uint32_t *)&dummy) != HAL_OK)
    {
      /* error detected. flag it and stop */
      result = BLT_FALSE;
      break;
    }
  }
  
  /* lock access to the flash device */
  HAL_FLASH_Lock();
  
  /* return the result */
  return result;
} /*** end of FlashErase ***/


/************************************************************************************//**
** \brief     Writes a checksum of the user program to non-volatile memory. This is
**            performed once the entire user program has been programmed. Through
**            the checksum, the bootloader can check if the programming session
**            was completed, which indicates that a valid user programming is
**            present and can be started.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashWriteChecksum(void)
{
  blt_int32u signature_checksum = 0;

  /* for the STM32 target we defined the checksum as the Two's complement value of the
   * sum of the first 7 exception addresses.
   *
   * Layout of the vector table:
   *    0x08000000 Initial stack pointer
   *    0x08000004 Reset Handler
   *    0x08000008 NMI Handler
   *    0x0800000C Hard Fault Handler
   *    0x08000010 MPU Fault Handler
   *    0x08000014 Bus Fault Handler
   *    0x08000018 Usage Fault Handler
   *
   *    signature_checksum = Two's complement of (SUM(exception address values))
   *
   *    the bootloader writes this 32-bit checksum value right after the vector table
   *    of the user program. note that this means one extra dummy entry must be added
   *    at the end of the user program's vector table to reserve storage space for the
   *    checksum.
   */

  /* first check that the bootblock contains valid data. if not, this means the
   * bootblock is not part of the reprogramming this time and therefore no
   * new checksum needs to be written
   */
  if (bootBlockInfo.base_addr == FLASH_INVALID_ADDRESS)
  {
    return BLT_TRUE;
  }

#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
  /* perform decryption of the bootblock, before calculating the checksum and writing it
   * to flash memory.
   */
  if (FlashCryptoDecryptDataHook(bootBlockInfo.data, FLASH_WRITE_BLOCK_SIZE) == BLT_FALSE)
  {
    return BLT_FALSE;
  }
#endif

  /* compute the checksum. note that the user program's vectors are not yet written
   * to flash but are present in the bootblock data structure at this point.
   */
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x00]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x04]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x08]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x0C]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x10]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x14]));
  signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x18]));
  signature_checksum  = ~signature_checksum; /* one's complement */
  signature_checksum += 1; /* two's complement */

  /* write the checksum */
  return FlashWrite(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET,
                    sizeof(blt_addr), (blt_int8u *)&signature_checksum);
} /*** end of FlashWriteChecksum ***/


/************************************************************************************//**
** \brief     Verifies the checksum, which indicates that a valid user program is
**            present and can be started.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashVerifyChecksum(void)
{
  blt_int32u signature_checksum = 0;

  /* verify the checksum based on how it was written by CpuWriteChecksum() */
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x04));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x08));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x0C));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x10));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x14));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x18));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET));
  /* sum should add up to an unsigned 32-bit value of 0 */
  if (signature_checksum == 0)
  {
    /* checksum okay */
    return BLT_TRUE;
  }
  /* checksum incorrect */
  return BLT_FALSE;
} /*** end of FlashVerifyChecksum ***/


/************************************************************************************//**
** \brief     Finalizes the flash driver operations. There could still be data in
**            the currently active block that needs to be flashed.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashDone(void)
{
  /* check if there is still data waiting to be programmed in the boot block */
  if (bootBlockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
    if (FlashWriteBlock(&bootBlockInfo) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }

  /* check if there is still data waiting to be programmed */
  if (blockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
    if (FlashWriteBlock(&blockInfo) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }
  /* still here so all is okay */
  return BLT_TRUE;
} /*** end of FlashDone ***/


/************************************************************************************//**
** \brief     Obtains the base address of the flash memory available to the user program.
**            This is basically the first address in the flashLayout table.
** \return    Base address.
**
****************************************************************************************/
blt_addr FlashGetUserProgBaseAddress(void)
{
  return flashLayout[0].sector_start;
} /*** end of FlashGetUserProgBaseAddress ***/


/************************************************************************************//**
** \brief     Copies data currently in flash to the block->data and sets the
**            base address.
** \param     block   Pointer to flash block info structure to operate on.
** \param     address Base address of the block data.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashInitBlock(tFlashBlockInfo *block, blt_addr address)
{
  /* check address alignment */
  if ((address % FLASH_WRITE_BLOCK_SIZE) != 0)
  {
    return BLT_FALSE;
  }
  /* make sure that we are initializing a new block and not the same one */
  if (block->base_addr == address)
  {
    /* block already initialized, so nothing to do */
    return BLT_TRUE;
  }
  /* set the base address and copies the current data from flash */
  block->base_addr = address;
  CpuMemCopy((blt_addr)block->data, address, FLASH_WRITE_BLOCK_SIZE);
  return BLT_TRUE;
} /*** end of FlashInitBlock ***/


/************************************************************************************//**
** \brief     Switches blocks by programming the current one and initializing the
**            next.
** \param     block   Pointer to flash block info structure to operate on.
** \param     base_addr Base address of the next block.
** \return    The pointer of the block info struct that is no being used, or a NULL
**            pointer in case of error.
**
****************************************************************************************/
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr)
{
  /* check if a switch needs to be made away from the boot block. in this case the boot
   * block shouldn't be written yet, because this is done at the end of the programming
   * session by FlashDone(), this is right after the checksum was written.
   */
  if (block == &bootBlockInfo)
  {
    /* switch from the boot block to the generic block info structure */
    block = &blockInfo;
  }
  /* check if a switch back into the bootblock is needed. in this case the generic block
   * doesn't need to be written here yet.
   */
  else if (base_addr == flashLayout[0].sector_start)
  {
    /* switch from the generic block to the boot block info structure */
    block = &bootBlockInfo;
    base_addr = flashLayout[0].sector_start;
  }
  else
  {
    /* need to switch to a new block, so program the current one and init the next */
    if (FlashWriteBlock(block) == BLT_FALSE)
    {
      return BLT_NULL;
    }
  }

  /* initialize tne new block when necessary */
  if (FlashInitBlock(block, base_addr) == BLT_FALSE)
  {
    return BLT_NULL;
  }

  /* still here to all is okay  */
  return block;
} /*** end of FlashSwitchBlock ***/


/************************************************************************************//**
** \brief     Programming is done per block. This function adds data to the block
**            that is currently collecting data to be written to flash. If the
**            address is outside of the current block, the current block is written
**            to flash an a new block is initialized.
** \param     block   Pointer to flash block info structure to operate on.
** \param     address Flash destination address.
** \param     data    Pointer to the byte array with data.
** \param     len     Number of bytes to add to the block.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashAddToBlock(tFlashBlockInfo *block, blt_addr address,
                                blt_int8u *data, blt_int32u len)
{
  blt_addr   current_base_addr;
  blt_int8u  *dst;
  blt_int8u  *src;

  /* determine the current base address */
  current_base_addr = (address/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;

  /* make sure the blockInfo is not uninitialized */
  if (block->base_addr == FLASH_INVALID_ADDRESS)
  {
    /* initialize the blockInfo struct for the current block */
    if (FlashInitBlock(block, current_base_addr) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }

  /* check if the new data fits in the current block */
  if (block->base_addr != current_base_addr)
  {
    /* need to switch to a new block, so program the current one and init the next */
    block = FlashSwitchBlock(block, current_base_addr);
    if (block == BLT_NULL)
    {
      return BLT_FALSE;
    }
  }

  /* add the data to the current block, but check for block overflow */
  dst = &(block->data[address - block->base_addr]);
  src = data;
  do
  {
    /* keep the watchdog happy */
    CopService();
    /* buffer overflow? */
    if ((blt_addr)(dst-&(block->data[0])) >= FLASH_WRITE_BLOCK_SIZE)
    {
      /* need to switch to a new block, so program the current one and init the next */
      block = FlashSwitchBlock(block, current_base_addr+FLASH_WRITE_BLOCK_SIZE);
      if (block == BLT_NULL)
      {
        return BLT_FALSE;
      }
      /* reset destination pointer */
      dst = &(block->data[0]);
    }
    /* write the data to the buffer */
    *dst = *src;
    /* update pointers */
    dst++;
    src++;
    /* decrement byte counter */
    len--;
  }
  while (len > 0);
  /* still here so all is good */
  return BLT_TRUE;
} /*** end of FlashAddToBlock ***/


/************************************************************************************//**
** \brief     Programs FLASH_WRITE_BLOCK_SIZE bytes to flash from the block->data
**            array.
** \param     block   Pointer to flash block info structure to operate on.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashWriteBlock(tFlashBlockInfo *block)
{
  blt_addr   prog_addr;
  uint64_t   prog_data;
  blt_int32u doubleword_cnt;
  blt_bool   result = BLT_TRUE;
  
#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
  #if (BOOT_NVM_CHECKSUM_HOOKS_ENABLE == 0)
  /* note that the bootblock is already decrypted in FlashWriteChecksum(), if the
   * internal checksum mechanism is used. Therefore don't decrypt it again.
   */
  if (block != &bootBlockInfo)
  #endif
  {
    /* perform decryption of the program data before writing it to flash memory. */
    if (FlashCryptoDecryptDataHook(block->data, FLASH_WRITE_BLOCK_SIZE) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }
#endif


  /* unlock access to the flash device */
  HAL_FLASH_Unlock();

  /* clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  
  /* program all double words in the block one by one */
  for (doubleword_cnt=0; doubleword_cnt<(FLASH_WRITE_BLOCK_SIZE/sizeof(uint64_t)); doubleword_cnt++)
  {
    prog_addr = block->base_addr + (doubleword_cnt * sizeof(uint64_t));
    prog_data = *(volatile uint64_t *)(&block->data[doubleword_cnt * sizeof(uint64_t)]);
    /* keep the watchdog happy */
    CopService();
    /* program the double word */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, prog_addr, prog_data) != HAL_OK)
    {
      /* error detected. flag it and stop */
      result = BLT_FALSE;
      break;
    }
    /* verify that the written data is actually there */
    if (*(volatile uint64_t *)prog_addr != prog_data)
    {
      result = BLT_FALSE;
      break;
    }
  }

  /* lock access to the flash device */
  HAL_FLASH_Lock();
  
  /* return the result */
  return result;
} /*** end of FlashWriteBlock ***/


/************************************************************************************//**
** \brief     Gets the page number of the address relative to the bank.
** \param     address Address in the flash bank.
** \return    The page of the given address: 0..255.
**
****************************************************************************************/
static blt_int32u FlashGetPage(blt_addr address)
{
  blt_int32u page = 0;
  
  /* is the address in the first bank? note that this can be either FLASH_BANK_1 or 
   * FLASH_BANK_2, due to the flash swap feature.
   */
  if (address < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    page = (address - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  /* address is in the second bank */
  else
  {
    page = (address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  
  return page;
} /*** end of FlashGetPage ***/


/************************************************************************************//**
** \brief     Obtains the bank of the given address. The 1024kb version of the flash
**            device contains 2 banks that can be swapped. This feature breaks the link
**            between a bank number and flash addresses. This function obtains the
**            bank number that is currently at the given address.
** \param     address Address in the flash bank.
** \return    The flash bank of the given address: FLASH_BANK_1 or FLASH_BANK_2. 
**
****************************************************************************************/
static blt_int32u FlashGetBank(blt_addr address)
{
  blt_int32u bank = FLASH_BANK_1;
	
	/* multiple banks is only supported on certain STM32L4xx derivatives. */
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || \
	  defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || \
	  defined (STM32L4A6xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || \
	  defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || \
	  defined (STM32L4S9xx)
  /* check flash bank mode selection bit to determine if banks 1 and 2 are swapped */
  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
  {
  	/* no bank swap active */
    if (address < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
  	/* bank swap active */
    if (address < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }
#endif	
  
  return bank;
} /*** end of FlashGetBank ***/


/*********************************** end of flash.c ************************************/
