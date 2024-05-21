#ifndef INC_W25QXX_H_
#define INC_W25QXX_H_

#define W25Q_SPI              hspi1         // spi handle, must be defined in platform*.c
#define W25Q_CS_Pin           GPIO_PIN_4    
#define W25Q_CS_GPIO_Port     GPIOA

#define W25Q_SPI_TX_DELAY_MS  2000
#define W25Q_SPI_RX_DELAY_MS  5000

// W25Q Instructions
#define W25Q_CMD_WREN           0x06  // Write enable
#define W25Q_CMD_WRDIS          0x04  // Write disable
#define W25Q_CMD_RDSR1          0x05  // Read Status Register-1
#define W25Q_CMD_RDSR2          0x35  // Read Status Register-2
#define W25Q_CMD_WRSR           0x01  // Write Status Register
#define W25Q_CMD_WRITE          0x02  // Page Program (Write to Memory)
#define W25Q_CMD_QWRITE         0x32  // Quad Page Program
#define W25Q_CMD_BER64          0xD8  // Block Erase (64KB)
#define W25Q_CMD_BER32          0x52  // Block Erase (32KB)
#define W25Q_CMD_SER            0x20  // Sector Erase (4KB)
#define W25Q_CMD_CER            0xC7  // Chip Erase
#define W25Q_CMD_ERSUS          0x75  // Erase Suspend
#define W25Q_CMD_ERRES          0x7A  // Erase Resume
#define W25Q_CMD_PWDN           0xB9  // Power-down
#define W25Q_CMD_HPM            0xA3  // High Performance Mode
#define W25Q_CMD_MBR            0xFF  // Mode Bit Reset
#define W25Q_CMD_PDHPMRES       0xAB  // Release Power down or HPM / Device ID
#define W25Q_CMD_MID            0x90  // Manufacturer/Device ID
#define W25Q_CMD_UID            0x4B  // Unique ID
#define W25Q_CMD_JEDECID        0x9F  // JEDEC ID

#define W25Q_CMD_READ           0x03  // Read from Memory

#define W25Q_CMD_RST_EN         0x66  // Reset enable
#define W25Q_CMD_RST            0x99  // Reset

#define W25Q_PAGE_LEN           256
#define W25Q_PAGES_PER_SECTOR   16
#define W25Q_SECTOR_LEN         (W25Q_PAGE_LEN * W25Q_PAGES_PER_SECTOR)

void W25Q_Reset(void);
uint16_t W25Q_Read_MID(void);
uint32_t W25Q_Read_JEDEC_ID(void);
uint16_t W25Q_Read_MID(void);
void W25Q_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
void W25Q_Erase_Sector(uint32_t sector);
void W25Q_Write_Page(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
void W25Q_Write_Chunk(uint32_t addr, uint8_t *data, uint32_t len);


#endif /* INC_W25QXX_H_ */
