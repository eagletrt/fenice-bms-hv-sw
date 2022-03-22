#include "bootloader.h"
#include "can.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "tim.h"
#include "main.h"

void BootLoaderInit() {
    void (*SysMemBootJump)(void) = (void (*)(void)) (*((uint32_t*)(BOOTLOADER_ADDR + 4)));
    uint32_t msp_addr = *(uint32_t*)BOOTLOADER_ADDR;

    HAL_CAN_DeInit(&hcan1);
    HAL_I2C_DeInit(&hi2c1);
    HAL_SPI_DeInit(&hspi1);
    HAL_SPI_DeInit(&hspi3);
    HAL_UART_DeInit(&huart1);
    HAL_TIM_Base_DeInit(&htim6);
    HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);

    __set_PRIMASK(1);
    __set_MSP(msp_addr);

    SysMemBootJump();

    while(1);
}