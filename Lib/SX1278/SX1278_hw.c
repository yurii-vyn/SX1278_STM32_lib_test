#include "main.h"
#include "stm32f1xx_hal.h"
#include "platform_f103.h"
#include "SX1278_hw.h"
#include "SX127x_reg.h"
#include <string.h>



void SX1278_hw_SetNSS(SX1278_hw_t *hw, int value)
{
	HAL_GPIO_WritePin(hw->nss.port, hw->nss.pin,
			(value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void SX1278_hw_select(SX1278_hw_t *hw)
{
  HAL_GPIO_WritePin(hw->nss.port, hw->nss.pin, GPIO_PIN_RESET);
}

void SX1278_hw_unselect(SX1278_hw_t *hw)
{
  HAL_GPIO_WritePin(hw->nss.port, hw->nss.pin, GPIO_PIN_SET);
}

void SX1278_hw_init(SX1278_hw_t *hw)
{
	SX1278_hw_unselect(hw);
	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_SET);
}

void SX1278_hw_reset(SX1278_hw_t *hw)
{
	SX1278_hw_unselect(hw);
	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_RESET);

	SX1278_hw_delay_ms(1);

	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_SET);

	SX1278_hw_delay_ms(100);
}

void SX1278_hw_spi_write_byte(SX1278_hw_t *hw, uint8_t byte)
{
  SX1278_hw_select(hw);
  HAL_SPI_Transmit(hw->spi, &byte, 1, SX1278_SPI_TIMEOUT);
  while (HAL_SPI_GetState(hw->spi) != HAL_SPI_STATE_READY){};
}

uint8_t SX1278_hw_spi_read_byte(SX1278_hw_t *hw)
{
	uint8_t txByte = 0x00;
	uint8_t rxByte = 0x00;

	SX1278_hw_SetNSS(hw, 0);
	HAL_SPI_TransmitReceive(hw->spi, &txByte, &rxByte, 1, SX1278_SPI_TIMEOUT);
	while (HAL_SPI_GetState(hw->spi) != HAL_SPI_STATE_READY)
		;
	return rxByte;
}

void SX1278_hw_delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

int SX1278_hw_GetDIO0(SX1278_hw_t *hw)
{
	return (HAL_GPIO_ReadPin(hw->dio0.port, hw->dio0.pin) == GPIO_PIN_SET);
}
