#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "W25Qxx.h"



extern SPI_HandleTypeDef W25Q_SPI;



static void W25Q_Delay(uint32_t time);
static void W25Q_select(void);
static void W25Q_unselect(void);
static void W25Q_SPI_Write(uint8_t *data, uint8_t len);
static void W25Q_SPI_Read(uint8_t *data, uint8_t len);
static void W25Q_WriteEnable(void);
static void W25Q_WriteDisable(void);
static uint32_t bytestowrite (uint32_t size, uint16_t offset);


static void W25Q_Delay(uint32_t time)
{
	HAL_Delay(time);
}

static void W25Q_select(void)
{
	HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_RESET);
}

static void W25Q_unselect(void)
{
	HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_SET);
}

static void W25Q_SPI_Write(uint8_t *data, uint8_t len)
{
	HAL_SPI_Transmit(&W25Q_SPI, data, len, W25Q_SPI_TX_DELAY_MS);	
}

static void W25Q_SPI_Read(uint8_t *data, uint8_t len)
{
	HAL_SPI_Receive(&W25Q_SPI, data, len, W25Q_SPI_RX_DELAY_MS);
}

static void W25Q_WriteEnable(void)
{
	uint8_t tData = W25Q_CMD_WREN;

  W25Q_select();
  W25Q_SPI_Write(&tData, 1);
	W25Q_unselect();
	W25Q_Delay(5);
}

static void W25Q_WriteDisable(void)
{
	uint8_t tData = W25Q_CMD_WRDIS;

  W25Q_select();
  W25Q_SPI_Write(&tData, 1);
	W25Q_unselect();
	W25Q_Delay(5);
}


/**************************************************************************************************/

/**
 * Reset chip before reading/writing
 * 
 * Before calling this function, make sure that W25Q_SPI is already configured and initialized from platform*.c.
 */
void W25Q_Reset(void)
{
	uint8_t tData[2];
	tData[0] = W25Q_CMD_RST_EN;  // enable Reset
	tData[1] = W25Q_CMD_RST;  // Reset

	W25Q_select();
	W25Q_SPI_Write(tData, 2);
	W25Q_unselect();
	W25Q_Delay(100);
}

uint32_t W25Q_Read_JEDEC_ID(void)
{
	uint8_t tData = W25Q_CMD_JEDECID;  // Read JEDEC ID
	uint8_t rData[3];

	W25Q_select();
	W25Q_SPI_Write(&tData, 1);
	W25Q_SPI_Read(rData, 3);
	W25Q_unselect();
	return ((rData[0]<<16)|(rData[1]<<8)|rData[2]);
}

uint16_t W25Q_Read_MID(void)
{
	uint8_t tData[4];
	tData[0] = W25Q_CMD_MID;
	tData[1] = 0;
	tData[2] = 0;
	tData[3] = 0;
	uint8_t rData[2];

	W25Q_select();
	W25Q_SPI_Write(tData, 4);
	W25Q_SPI_Read(rData, 2);
	W25Q_unselect();
	return ((rData[0]<<8)|rData[1]);
}

void W25Q_Erase_Sector(uint32_t sector)
{
	uint8_t tData[6];
	uint32_t memAddr = sector*16*256;   // Each sector contains 16 pages * 256 bytes (4K bytes)

	W25Q_WriteEnable();

	tData[0] = 0x20;  								// Erase sector
	tData[1] = (memAddr>>16)&0xFF;  	// MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; 				// LSB of the memory Address

	W25Q_select();
	W25Q_SPI_Write(tData, 4);
	W25Q_unselect();

	W25Q_Delay(450);  // 450ms delay for sector erase
	W25Q_WriteDisable();
}

void W25Q_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
{
	uint8_t tData[5];
	uint32_t memAddr = (startPage*256) + offset;

	tData[0] = 0x03;  // enable Read
	tData[1] = (memAddr>>16)&0xFF;  // MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; // LSB of the memory Address

	W25Q_select();  // pull the CS Low
	W25Q_SPI_Write(tData, 4);  // send read instruction along with the 24 bit memory address
	W25Q_SPI_Read(rData, size);  // Read the data
	W25Q_unselect();  // pull the CS High
}

static uint32_t bytestowrite (uint32_t size, uint16_t offset)
{
	if ((size+offset)<256) return size;
	else return 256-offset;
}

void W25Q_Write_Page(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
{
	uint8_t tData[266];
	uint32_t startPage = page;
	uint32_t endPage  = startPage + ((size+offset-1)/256);
	uint32_t numPages = endPage-startPage+1;
	uint16_t startSector  = startPage/16;
	uint16_t endSector  = endPage/16;
	uint16_t numSectors = endSector-startSector+1;

	for (uint16_t i = 0; i < numSectors; i++){
		W25Q_Erase_Sector(startSector++);
	}

	uint32_t dataPosition = 0;

	// write the data
	for (uint32_t i = 0; i < numPages; i++){
		uint32_t memAddr = (startPage*256)+offset;
		uint16_t bytesremaining  = bytestowrite(size, offset);
		W25Q_WriteEnable();
    
		uint32_t indx = 0;

		tData[0] = 0x02;  // page program
		tData[1] = (memAddr>>16)&0xFF;  // MSB of the memory Address
		tData[2] = (memAddr>>8)&0xFF;
		tData[3] = (memAddr)&0xFF; // LSB of the memory Address

		indx = 4;

		uint16_t bytestosend  = bytesremaining + indx;

		for (uint16_t i=0; i<bytesremaining; i++){
			tData[indx++] = data[i+dataPosition];
		}

		W25Q_select();
		W25Q_SPI_Write(tData, bytestosend);
		W25Q_unselect();

		startPage++;
		offset = 0;
		size = size-bytesremaining;
		dataPosition = dataPosition+bytesremaining;
		W25Q_Delay(5);
		W25Q_WriteDisable();
	}
}

void W25Q_Write_Chunk(uint32_t addr, uint8_t *data, uint32_t len)
{
	uint8_t tData[32];

	W25Q_WriteEnable();

	tData[0] = 0x02;  // page program
	tData[1] = (addr>>16)&0xFF;  // MSB of the memory Address
	tData[2] = (addr>>8)&0xFF;
	tData[3] = (addr)&0xFF; // LSB of the memory Address
	memcpy(tData+4, data, len);

	W25Q_select();
	W25Q_SPI_Write(tData, len+4);
	W25Q_unselect();

	W25Q_Delay(5);
	W25Q_WriteDisable();
}