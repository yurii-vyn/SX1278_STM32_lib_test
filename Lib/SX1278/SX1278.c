#include "SX1278.h"
#include <string.h>

/**
 * \brief Read byte from LoRa module
 *
 * Reads data from LoRa module from given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address from which data will be read
 *
 * \return              Read data
 */
uint8_t SX1278_SPIRead(SX1278_t *module, uint8_t addr) {
	uint8_t tmp;
	SX1278_hw_spi_write_byte(module->hw, addr);
	tmp = SX1278_hw_spi_read_byte(module->hw);
	SX1278_hw_unselect(module->hw);
	return tmp;
}

/**
 * \brief Write byte to LoRa module
 *
 * Writes data to LoRa module under given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address under which data will be written
 * \param[in]  cmd 		Data to write
 */
void SX1278_SPIWrite(SX1278_t *module, uint8_t addr, uint8_t cmd) {
	SX1278_hw_select(module->hw);
	SX1278_hw_spi_write_byte(module->hw, addr | 0x80);
	SX1278_hw_spi_write_byte(module->hw, cmd);
	SX1278_hw_unselect(module->hw);
}

/**
 * \brief Read data from LoRa module
 *
 * Reads data from LoRa module from given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address from which data will be read
 * \param[out] rxBuf    Pointer to store read data
 * \param[in]  length   Number of bytes to read
 */
void SX1278_SPIBurstRead(SX1278_t *module, uint8_t addr, uint8_t *rxBuf, uint8_t length)
{
	uint8_t i;
	if (length <= 1) {
		return;
	} else {
		SX1278_hw_select(module->hw);
		SX1278_hw_spi_write_byte(module->hw, addr);
#ifndef SX1278_SPI_USE_DMA
		for (i = 0; i < length; i++) {
			*(rxBuf + i) = SX1278_hw_spi_read_byte(module->hw);
		}
#else
    SX1278_hw_spi_read_dma(module->hw, rxBuf, length);
#endif
		SX1278_hw_unselect(module->hw);
	}
}

/**
 * \brief Write data to LoRa module
 *
 * Writes data to LoRa module under given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address under which data will be written
 * \param[in]  txBuf    Pointer to data
 * \param[in]  length   Number of bytes to write
 */
void SX1278_SPIBurstWrite(SX1278_t *module, uint8_t addr, uint8_t *txBuf, uint8_t length)
{
	unsigned char i;
	if (length <= 1) {
		return;
	} else {
    SX1278_hw_select(module->hw);
		SX1278_hw_spi_write_byte(module->hw, addr | 0x80);
#ifndef SX1278_SPI_USE_DMA
		for (i = 0; i < length; i++) {
			SX1278_hw_spi_write_byte(module->hw, *(txBuf + i));
		}
#else
    SX1278_hw_spi_write_dma(module->hw, txBuf, length);
#endif
    SX1278_hw_unselect(module->hw);
	}
}

/**
 * \brief Set carrier frequency
*/
void SX1278_set_frequency(SX1278_t *module, uint64_t freq)
{
	module->frequency = freq;

	uint64_t sfreq = module->frequency;
	sfreq = (uint64_t)(sfreq << 19);
	sfreq = (uint64_t)(sfreq / 32000000);
	// uint64_t sfreq = ((uint64_t) module->frequency << 19) / 32000000;
	uint8_t freq_reg[3];
	freq_reg[0] = (uint8_t) (sfreq >> 16);
	freq_reg[1] = (uint8_t) (sfreq >> 8);
	freq_reg[2] = (uint8_t) (sfreq >> 0);

	SX1278_sleep(module);																								// enter sleep mode before writing anything to registers
	SX1278_SPIBurstWrite(module, LR_RegFrMsb, (uint8_t*) freq_reg, 3);	// write data
	SX1278_standby(module);																							// go back to satandby
}

/**
 * \brief Set spreading factor
*/
void SX1278_set_sf(SX1278_t *module, uint8_t sf)
{
	module->LoRa_SF = sf;

	SX1278_sleep(module); 																			// enter sleep mode before writing anything to registers
	SX1278_modem_config(module);
	SX1278_standby(module);																			// go back to satandby
}

/**
 * \brief Set bandwidth
*/
void SX1278_set_bw(SX1278_t *module, uint8_t bw)
{
	module->LoRa_BW = bw;

	SX1278_sleep(module); 																			// enter sleep mode before writing anything to registers
	SX1278_modem_config(module);
	SX1278_standby(module);																			// go back to satandby
}

/**
 * \brief Set coding rate
*/
void SX1278_set_cr(SX1278_t *module, uint8_t cr)
{
	module->LoRa_CR = cr;

	SX1278_sleep(module); 																			// enter sleep mode before writing anything to registers
	SX1278_modem_config(module);
	SX1278_standby(module);																			// go back to satandby
}

/**
 * \brief Enable or disable CRC
*/
void SX1278_set_crc(SX1278_t *module, uint8_t crc_en)
{
	module->LoRa_CRC_sum = crc_en;

	SX1278_sleep(module); 																			// enter sleep mode before writing anything to registers
	SX1278_modem_config(module);
	SX1278_standby(module);																			// go back to satandby
}

/**
 * \brief Set sync word
*/
void SX1278_set_sync_word(SX1278_t *module, uint8_t s_word)
{
	module->sync_word = s_word;

	SX1278_sleep(module); 																			// enter sleep mode before writing anything to registers
	SX1278_SPIWrite(module, RegSyncWord, module->sync_word);		// set sync word
	SX1278_standby(module);																			// go back to satandby
}

void SX1278_set_fhss_period(SX1278_t *module, uint8_t period)
{
  module->FHSS_period = period;

  SX1278_sleep(module); 
  SX1278_SPIWrite(module, LR_RegHopPeriod, period);
  SX1278_standby(module);	
}

/**
 * \brief Write modem modem config registers (1-3)
*/
void SX1278_modem_config(SX1278_t *module)
{
	if (SX1278_SpreadFactor[module->LoRa_SF] == 6) {	// SFactor=6
		uint8_t tmp;
		SX1278_SPIWrite(module,
		LR_RegModemConfig1,
				((SX1278_LoRaBandwidth[module->LoRa_BW] << 4)
						+ (SX1278_CodingRate[module->LoRa_CR] << 1) + 0x01)); //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

		SX1278_SPIWrite(module,
		LR_RegModemConfig2,
				((SX1278_SpreadFactor[module->LoRa_SF] << 4)
						+ (SX1278_CRC_Sum[module->LoRa_CRC_sum] << 2) + 0x03));

		tmp = SX1278_SPIRead(module, 0x31);
		tmp &= 0xF8;
		tmp |= 0x05;
		SX1278_SPIWrite(module, 0x31, tmp);
		SX1278_SPIWrite(module, 0x37, 0x0C);
	} else {
		SX1278_SPIWrite(module,
		LR_RegModemConfig1,
				((SX1278_LoRaBandwidth[module->LoRa_BW] << 4)
						+ (SX1278_CodingRate[module->LoRa_CR] << 1) + 0x00)); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

		SX1278_SPIWrite(module,
		LR_RegModemConfig2,
				((SX1278_SpreadFactor[module->LoRa_SF] << 4)
						+ (SX1278_CRC_Sum[module->LoRa_CRC_sum] << 2) + 0x00)); //SFactor &  LNA gain set by the internal AGC loop
	}

	SX1278_SPIWrite(module, LR_RegModemConfig3, 	0x04);
}

/**
 * \brief Configure LoRa module
 *
 * Configure LoRa module according to parameters stored in
 * module structure.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void SX1278_config(SX1278_t *module)
{
	SX1278_sleep(module); 		// enter sleep mode before writing anything to registers
	SX1278_enableLoRa(module);
	//SX1278_SPIWrite(module, 0x5904); //?? Change digital regulator form 1.6V to 1.47V: see errata note

	uint64_t sfreq = module->frequency;			// set frequency
	sfreq = (uint64_t)(sfreq << 19);
	sfreq = (uint64_t)(sfreq / 32000000);
	uint8_t freq_reg[3];
	freq_reg[0] = (uint8_t) (sfreq >> 16);
	freq_reg[1] = (uint8_t) (sfreq >> 8);
	freq_reg[2] = (uint8_t) (sfreq >> 0);
	SX1278_SPIBurstWrite(module, LR_RegFrMsb, (uint8_t*) freq_reg, 3);

	SX1278_SPIWrite(module, RegSyncWord, module->sync_word);							// set sync word
	SX1278_SPIWrite(module, LR_RegPaConfig, SX1278_Power[module->power]); // set output power
	SX1278_SPIWrite(module, LR_RegOcp, 0x0B);			// RegOcp, Close Ocp
	SX1278_SPIWrite(module, LR_RegLna, 0x23);			// RegLNA, High & LNA Enable
	
	SX1278_modem_config(module);

	SX1278_SPIWrite(module, LR_RegSymbTimeoutLsb, 0x08); 		// RegSymbTimeoutLsb Timeout = 0x3FF(Max)
	SX1278_SPIWrite(module, LR_RegPreambleMsb, 		0x00); 		// RegPreambleMsb
	SX1278_SPIWrite(module, LR_RegPreambleLsb, 		module->preamble); 			// RegPreambleLsb 8+4=12byte Preamble
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING2, 	0x01); 		// RegDioMapping2 DIO5=00, DIO4=01
	module->readBytes = 0;
	SX1278_standby(module); 																// Enter standby mode
}

/**
 * \brief Enter standby mode
 *
 * Enters standby mode.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void SX1278_standby(SX1278_t *module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x09);
	module->status = STANDBY;
	SX1278_hw_delay_ms(20);
}

/**
 * \brief Enter sleep mode
 *
 * Enters sleep mode.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void SX1278_sleep(SX1278_t *module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x08);
	module->status = SLEEP;
	SX1278_hw_delay_ms(20);
}

/**
 * \brief Enable LoRa mode
 *
 * Module supports different operation modes.
 * To use LoRa operation mode one has to enter this
 * particular mode to transmit and receive data
 * using LoRa.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void SX1278_enableLoRa(SX1278_t *module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x88);
}

/**
 * \brief Clear IRQ
 *
 * Clears interrupt flags.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void SX1278_clearLoRaIrq(SX1278_t *module) {
	SX1278_SPIWrite(module, LR_RegIrqFlags, 0xFF);
}

/**
 * \brief Enter reception mode
 *
 * Entry reception mode
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be received
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_LoRaSetRxMode(SX1278_t *module, uint8_t length, uint32_t timeout) {
	uint8_t addr;

	module->packetLength = length;

	// SX1278_config(module);		                                      // Set base parameters
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x84);	                    // Normal and RX
	SX1278_SPIWrite(module, LR_RegHopPeriod, module->FHSS_period);	  // Set FHSS period
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x01);                // DIO=00, DIO1=00, DIO2=00, DIO3=01
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0x3F);                // Open RxDone interrupt & Timeout
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegPayloadLength, length);             // Payload Length
	addr = SX1278_SPIRead(module, LR_RegFifoRxBaseAddr);              // Read RxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr);                 // RxBaseAddr->FiFoAddrPtr
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8d);	                    // Mode // Low Frequency Mode
	//SX1278_SPIWrite(module, LR_RegOpMode,0x05);	                    // Continuous Rx Mode // High Frequency Mode
	module->readBytes = 0;

	while (1) {
		if ((SX1278_SPIRead(module, LR_RegModemStat) & 0x04) == 0x04) {	//Rx-on going RegModemStat
			module->status = RX;
			return 1;
		}
		if (--timeout == 0) {
			SX1278_hw_reset(module->hw);
			SX1278_config(module);
			return 0;
		}
		SX1278_hw_delay_ms(1);
	}
}

/**
 * \brief Read data
 *
 * Read data and store it in module's RX buffer
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_LoRaRxPacket(SX1278_t *module) {
	unsigned char addr;
	unsigned char packet_size;
	module->readBytes = 0;

	if (SX1278_hw_GetDIO0(module->hw)) {
		memset(module->rxBuffer, 0x00, SX1278_MAX_PACKET);

		addr = SX1278_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
		SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr

		if (module->LoRa_SF == SX1278_LORA_SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
			packet_size = module->packetLength;
		} else {
			packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
		}

		SX1278_SPIBurstRead(module, 0x00, module->rxBuffer, packet_size);
		module->readBytes = packet_size;
		SX1278_clearLoRaIrq(module);
	}
	return module->readBytes;
}

/**
 * \brief Enter transmitter mode
 *
 * Entry transmitter mode
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_LoRaSetTxMode(SX1278_t *module, uint8_t length, uint32_t timeout) {
	uint8_t addr = 0;
	uint8_t temp = 0;

	module->packetLength = length;
	// SX1278_config(module);                                         // set base parameters
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x87);	                    // Tx for 20dBm 0x87
	SX1278_SPIWrite(module, LR_RegHopPeriod, module->FHSS_period);    // RegHopPeriod
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x41);                // DIO0=01, DIO1=00, DIO2=00, DIO3=01
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0xF7);                // Open TxDone interrupt
	SX1278_SPIWrite(module, LR_RegPayloadLength, length);             // RegPayloadLength 21byte
	addr = SX1278_SPIRead(module, LR_RegFifoTxBaseAddr);              // RegFiFoTxBaseAddr
  module->fifo_addr = addr;
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr);                 // RegFifoAddrPtr

	while (1) {
		temp = SX1278_SPIRead(module, LR_RegPayloadLength);
		if (temp == length) {
			module->status = TX;
			return 1;
		}

		if (--timeout == 0) {
			SX1278_hw_reset(module->hw);
			SX1278_config(module);
			return 0;
		}
	}
}

/**
 * \brief Send data
 *
 * Transmit data
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  txBuf    Data buffer with data to be sent
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_LoRaTxPacket(SX1278_t *module, uint8_t *txBuffer, uint8_t length, uint32_t timeout)
{
  SX1278_SPIBurstWrite(module, module->fifo_addr, txBuffer, length);
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode
	
	while (1) {
		if (SX1278_hw_GetDIO0(module->hw)) { //if(Get_NIRQ()) //Packet send over
			SX1278_SPIRead(module, LR_RegIrqFlags);
			SX1278_clearLoRaIrq(module); //Clear irq
			SX1278_standby(module); //Entry Standby mode
			return 1;
		}

		if (--timeout == 0) {
			SX1278_hw_reset(module->hw);
			SX1278_config(module);
			return 0;
		}
		SX1278_hw_delay_ms(1);
	}
}

/**
 * \brief Initialize LoRa module
 *
 * Initialize LoRa module and initialize LoRa structure.
 *
 * \param[in]  module	    	Pointer to LoRa structure
 * \param[in]  frequency    Frequency in [Hz]
 * \param[in]  power        Power level, accepts SX1278_POWER_*
 * \param[in]  LoRa_SF      LoRa spread rate, accepts SX1278_LORA_SF_*
 * \param[in]  LoRa_BW      LoRa bandwidth, accepts SX1278_LORA_BW_*
 * \param[in]  LoRa_CR      LoRa coding rate, accepts SX1278_LORA_CR_*
 * \param[in]  LoRa_CRC_sum Hardware CRC check, SX1278_LORA_CRC_EN or
 *                          SX1278_LORA_CRC_DIS
 * \param[in]  packetLength Package length, no more than 256 bytes
 */
void SX1278_init(SX1278_t *module, uint64_t frequency, uint8_t power,
									uint8_t LoRa_SF, uint8_t LoRa_BW, uint8_t LoRa_CR,
									uint8_t LoRa_CRC_sum, uint8_t preambleLength, 
                  uint8_t sync_w, uint8_t fhss_period, uint8_t packetLength)
{
	SX1278_hw_init(module->hw);
	module->frequency = frequency;
	module->power = power;
	module->LoRa_SF = LoRa_SF;
	module->LoRa_BW = LoRa_BW;
	module->LoRa_CR = LoRa_CR;
	module->LoRa_CRC_sum = LoRa_CRC_sum;
	module->preamble = (uint8_t)(preambleLength-4);
	module->sync_word = sync_w;
  module->FHSS_period = fhss_period;
	module->packetLength = packetLength;
	SX1278_config(module);
}

/**
 * \brief Enter transmitter mode and send data
 *
 * Entry transmitter mode and send data.
 * Combination of SX1278_LoRaEntryTx() and SX1278_LoRaTxPacket().
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  txBuf    Data buffer with data to be sent
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entered TX mode and sent data
 *             0 if timeout was exceeded
 */
int SX1278_transmit(SX1278_t *module, uint8_t *txBuf, uint8_t length,
		uint32_t timeout)
{
	if (SX1278_LoRaSetTxMode(module, length, timeout)) {
		return SX1278_LoRaTxPacket(module, txBuf, length, timeout);
	}
	return 0;
}

/**
 * \brief Enter reception mode
 *
 * Same as SX1278_LoRaEntryRx()
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be received
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_receive(SX1278_t *module, uint8_t length, uint32_t timeout) {
	return SX1278_LoRaSetRxMode(module, length, timeout);
}

/**
 * \brief Returns number of received data
 *
 * Returns the number of received data which are
 * held in internal buffer.
 * Same as SX1278_LoRaRxPacket().
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_available(SX1278_t *module) {
	return SX1278_LoRaRxPacket(module);
}

/**
 * \brief Read received data to buffer
 *
 * Reads data from internal buffer to external
 * buffer. Reads exactly number of bytes which are stored in
 * internal buffer.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[out] rxBuf    External buffer to store data.
 *                      External buffer is terminated with '\0'
 *                      character
 * \param[in]  length   Length of message to be received
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_read(SX1278_t *module, uint8_t *rxBuf, uint8_t length) {
	if (length != module->readBytes)
		length = module->readBytes;
	memcpy(rxBuf, module->rxBuffer, length);
	rxBuf[length] = '\0';
	module->readBytes = 0;
	return length;
}

/**
 * \brief Returns RSSI (LoRa)
 *
 * Returns RSSI in LoRa mode.
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     RSSI value
 */
uint8_t SX1278_RSSI_LoRa(SX1278_t *module) {
	uint32_t temp = 10;
	temp = SX1278_SPIRead(module, LR_RegRssiValue); //Read RegRssiValue, Rssi value
	temp = temp + 127 - 137; //127:Max RSSI, 137:RSSI offset
	return (uint8_t) temp;
}

/**
 * \brief Returns RSSI
 *
 * Returns RSSI (general mode).
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     RSSI value
 */
uint8_t SX1278_RSSI(SX1278_t *module) {
	uint8_t temp = 0xff;
	temp = SX1278_SPIRead(module, RegRssiValue);
	temp = 127 - (temp >> 1);	//127:Max RSSI
	return temp;
}
