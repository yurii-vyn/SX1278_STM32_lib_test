#include <stdio.h>
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "radio_tx.h"
#include "SX1278.h"
#include "platform_f103.h"
#include "usb_helpers.h"
#include "usbd_cdc_if.h"


extern SPI_HandleTypeDef SX1278_SPI;


SX1278_hw_t   SX1278_hw;
SX1278_t      SX1278;
uint8_t       radio_txrx_buffer[SX1278_PACKET_LEN];
uint8_t       radio_status = SX1278_STATUS_ERROR;
uint32_t      radio_task_timer = 0;
uint8_t       task_status = SX1278_STATUS_ERROR;
uint8_t       radio_txrx_len = 0;

uint8_t       radio_test_tx_enable = RADIO_TX_TEST_DISABLED;

/**
 * SX1278 initialization
 * 
 * Initialize GPIO and SPI before calling this function
*/
uint8_t radio_init(void)
{
  SX1278_hw_delay_ms(100);  // delay before start (unnecessary)
  SX1278_hw.dio0.port =   SX1278_DIO0_GPIO_Port; 
	SX1278_hw.dio0.pin =    SX1278_DIO0_Pin;
	SX1278_hw.nss.port =    SX1278_NSS_GPIO_Port;
	SX1278_hw.nss.pin =     SX1278_NSS_Pin;
	SX1278_hw.reset.port =  SX1278_RESET_GPIO_Port;
	SX1278_hw.reset.pin =   SX1278_RESET_Pin;
	SX1278_hw.spi =         &SX1278_SPI;

	SX1278.hw = &SX1278_hw;

  // initialize SX with basic parameters
  SX1278_init(&SX1278, SX1278_FREQ, SX1278_PWR, SX1278_SF,
	              SX1278_BW, SX1278_CR, SX1278_LORA_CRC_EN, SX1278_PR_LEN, SX1278_LORA_SW, SX1278_FHSS, SX1278_PACKET_LEN);
  // wait for proper startup
  SX1278_hw_delay_ms(500); 
#ifdef RADIO_MODE_TX
  // switch to tx mode
  radio_status = SX1278_LoRaSetTxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif

#ifdef RADIO_MODE_RX
  // switch to rx mode
  radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
  if(radio_status != SX1278_STATUS_OK){
    cdc_msg_print("TX init ERROR\r\n");
  }

  return radio_status;
}

void radio_tx_test_enable(void)
{
  radio_test_tx_enable = RADIO_TX_TEST_ENABLED;
}

void radio_tx_test_disable(void)
{
  radio_test_tx_enable = RADIO_TX_TEST_DISABLED;
}

/**
 * Transmit test message 
*/
uint8_t radio_tx_test_transmit(uint32_t sys_tick_ms)
{
  if(radio_test_tx_enable == RADIO_TX_TEST_ENABLED){
    // radio_txrx_len = sprintf((char*)radio_txrx_buffer, "Test message...");
    memset(radio_txrx_buffer, 0x00, SX1278_PACKET_LEN);
    radio_txrx_len = sprintf((char*)radio_txrx_buffer, "T:%lu\r\n", sys_tick_ms);
    if(radio_txrx_len <= SX1278_PACKET_LEN){
      // radio_status = SX1278_LoRaTxPacket(&SX1278, radio_txrx_buffer, tx_len, SX1278_TIMEOUT_MS);
      radio_status = SX1278_transmit(&SX1278, radio_txrx_buffer, radio_txrx_len, SX1278_TIMEOUT_MS);
    }
  }

  return radio_status;
}

void radio_tx_idle_task(uint32_t sys_tick_ms)
{
  if(sys_tick_ms >= radio_task_timer){
    radio_task_timer = sys_tick_ms + RADIO_TX_TASK_PERIOD;

    task_status = radio_tx_test_transmit(sys_tick_ms);

    if(task_status == SX1278_STATUS_OK){
      // cdc_msg_print("Tx OK\r\n");
    }else{
      cdc_msg_print("Tx ERROR\r\n");
    }
  }
}

void radio_rx_idle_task(uint32_t sys_tick_ms)
{
  if(sys_tick_ms >= radio_task_timer){
    radio_task_timer = sys_tick_ms + 20;
    radio_txrx_len = SX1278_LoRaRxPacket(&SX1278);
    if((radio_txrx_len > 0) && (radio_txrx_len <= SX1278_PACKET_LEN) && (radio_txrx_len <= APP_TX_DATA_SIZE)){
      memset(radio_txrx_buffer, 0x00, SX1278_PACKET_LEN);
      SX1278_read(&SX1278, radio_txrx_buffer, radio_txrx_len);
      cdc_msg_print("%s", radio_txrx_buffer);
      // task_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
    }
    task_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
  }
}

void radio_tx_set_frequency(uint64_t freq)
{
  SX1278_set_frequency(&SX1278, freq);
#ifdef RADIO_MODE_RX
  radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
}

void radio_tx_set_sf(uint8_t data_in)
{
  if((data_in >= RADIO_SF_MIN) && (data_in <= RADIO_SF_MAX)){
    SX1278_set_sf(&SX1278, (data_in - RADIO_SF_MIN));   // if data_in == 6 (spreading factor 6), set index to 0; 7-1, 8-2, ...
#ifdef RADIO_MODE_RX
    radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
  }else{
    cdc_msg_print("ERROR: SF OOR\r\n"); // error
  }
}

void radio_tx_set_cr(uint8_t data_in)
{
  if((data_in >= RADIO_CR_MIN) && (data_in <= RADIO_CR_MAX)){
    SX1278_set_cr(&SX1278, (data_in - RADIO_CR_MIN));
#ifdef RADIO_MODE_RX
    radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
  }else{
    cdc_msg_print("ERROR: CR OOR\r\n"); // error
  }
}

void radio_tx_set_crc(uint8_t crc)
{
  if((crc == 0) || (crc == 1)){
    SX1278_set_crc(&SX1278,  crc);
#ifdef RADIO_MODE_RX
    radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
  }else{
    cdc_msg_print("ERROR: CRC OOR\r\n"); // error
  }
}

void radio_tx_set_bw(uint16_t data_in)
{
  uint8_t bw = 0;
  uint8_t bw_valid = 1;

  switch(data_in)
  {
  case RADIO_BW_7KHZ:
    bw = SX1278_LORA_BW_7_8KHZ;
    break;
  case RADIO_BW_10KHZ:
    bw = SX1278_LORA_BW_10_4KHZ;
    break;
  case RADIO_BW_15KHZ:
    bw = SX1278_LORA_BW_15_6KHZ;
    break;
  case RADIO_BW_20KHZ:
    bw = SX1278_LORA_BW_20_8KHZ;
    break;
  case RADIO_BW_31KHZ:
    bw = SX1278_LORA_BW_31_2KHZ;
    break;
  case RADIO_BW_41KHZ:
    bw = SX1278_LORA_BW_41_7KHZ;
    break;
  case RADIO_BW_62KHZ:
    bw = SX1278_LORA_BW_62_5KHZ;
    break;
  case RADIO_BW_125KHZ:
    bw = SX1278_LORA_BW_125KHZ;
    break;
  case RADIO_BW_250KHZ:
    bw = SX1278_LORA_BW_250KHZ;
    break;
  case RADIO_BW_500KHZ:
    bw = SX1278_LORA_BW_500KHZ;
    break;
  default:
    bw_valid = 0;
    cdc_msg_print("ERROR: BW OOR\r\n"); // error
    break;
  }

  if(bw_valid == 1){
    SX1278_set_bw(&SX1278, bw);
#ifdef RADIO_MODE_RX
    radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
  }
}

uint64_t radio_tx_get_frequency(void)
{
  return SX1278.frequency;
}

uint8_t radio_tx_get_sf(void)
{
  return SX1278.LoRa_SF+RADIO_SF_MIN;
}

uint8_t radio_tx_get_cr(void)
{
  return SX1278.LoRa_CR;
}

uint8_t radio_tx_get_crc(void)
{
  return SX1278.LoRa_CRC_sum;
}

uint8_t radio_tx_get_bw(void)
{
  return SX1278.LoRa_BW;
}

uint8_t radio_tx_custom_msg(uint8_t* msg, uint8_t len)
{
  radio_status = SX1278_transmit(&SX1278, msg, len, SX1278_TIMEOUT_MS);

  return radio_status;
}

void radio_tx_set_fhss_period(uint8_t period)
{
  SX1278_set_fhss_period(&SX1278, period);
#ifdef RADIO_MODE_RX
  radio_status = SX1278_LoRaSetRxMode(&SX1278, SX1278_PACKET_LEN, SX1278_TIMEOUT_MS);
#endif
}

uint8_t radio_tx_get_fhss_period(void)
{
  return SX1278.FHSS_period;
}