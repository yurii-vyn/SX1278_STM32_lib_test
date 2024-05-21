#ifndef _SX1278_HW_H_
#define _SX1278_HW_H_

#include <stdint.h>

#define SX1278_SPI_TIMEOUT 1000
#define SX1278_SPI_USE_DMA

typedef struct {
	int pin;
	void *port;
} SX1278_hw_dio_t;

typedef struct {
	SX1278_hw_dio_t reset;
	SX1278_hw_dio_t dio0;
	SX1278_hw_dio_t nss;
	void *spi;
} SX1278_hw_t;

/**
 * \brief Initialize hardware layer
 *
 * Clears NSS and resets LoRa module.
 *
 * \param[in]   hw 		Pointer to hardware structure
 */
void SX1278_hw_init(SX1278_hw_t *hw);

/**
 * \brief Control NSS
 *
 * Clears and sets NSS according to passed value.
 *
 * \param[in]   hw 		Pointer to hardware structure.
 * \param[in]   value   1 sets NSS high, other value sets NSS low.
 */
void SX1278_hw_SetNSS(SX1278_hw_t *hw, int value);

/**
 * NSS select
*/
void SX1278_hw_select(SX1278_hw_t *hw);

/**
 * NSS unselect
*/
void SX1278_hw_unselect(SX1278_hw_t *hw);

/**
 * \brief Resets LoRa module
 *
 * Resets LoRa module.
 *
 * \param[in]   hw 		Pointer to hardware structure
 */
void SX1278_hw_reset(SX1278_hw_t *hw);

/**
 * \brief Send command via SPI.
 *
 * Send single byte via SPI interface.
 *
 * \param[in]   hw 		Pointer to hardware structure
 * \param[in]   cmd		Command
 */
void SX1278_hw_spi_write_byte(SX1278_hw_t *hw, uint8_t cmd);

/**
 * \brief Reads data via SPI
 *
 * Reads data via SPI interface.
 *
 * \param[in]   hw 		Pointer to hardware structure
 *
 * \return				Read value
 */
uint8_t SX1278_hw_spi_read_byte(SX1278_hw_t *hw);

/**
 * \brief ms delay
 *
 * Milisecond delay.
 *
 * \param[in]   msec 		Number of milliseconds to wait
 */
void SX1278_hw_delay_ms(uint32_t ms);

/**
 * \brief Reads DIO0 state
 *
 * Reads LoRa DIO0 state using GPIO.
 *
 * \param[in]   hw 		Pointer to hardware structure
 *
 * \return				0 if DIO0 low, 1 if DIO high
 */
int SX1278_hw_GetDIO0(SX1278_hw_t *hw);

#endif // _SX1278_HW_H_

