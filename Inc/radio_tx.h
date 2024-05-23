#ifndef _RADIO_TX_H_
#define _RADIO_TX_H_

#define RADIO_TX_TASK_PERIOD    200

#define SX1278_NSS_GPIO_Port    GPIOA
#define SX1278_NSS_Pin          GPIO_PIN_4
#define SX1278_DIO0_GPIO_Port   GPIOA
#define SX1278_DIO0_Pin         GPIO_PIN_3
#define SX1278_RESET_GPIO_Port  GPIOB
#define SX1278_RESET_Pin        GPIO_PIN_0
#define SX1278_SPI              hspi1

#define SX1278_TIMEOUT_MS       500
#define SX1278_STATUS_OK        1
#define SX1278_STATUS_ERROR     0

#define RADIO_TX_TEST_ENABLED   1
#define RADIO_TX_TEST_DISABLED  0

#define RADIO_SF_MIN            6
#define RADIO_SF_MAX            12
#define RADIO_CR_MIN            5
#define RADIO_CR_MAX            8

#define RADIO_BW_7KHZ           7
#define RADIO_BW_10KHZ          10
#define RADIO_BW_15KHZ          15
#define RADIO_BW_20KHZ          20
#define RADIO_BW_31KHZ          31
#define RADIO_BW_41KHZ          41
#define RADIO_BW_62KHZ          62
#define RADIO_BW_125KHZ         125
#define RADIO_BW_250KHZ         250
#define RADIO_BW_500KHZ         500

// LoRa parameters
#define SX1278_FREQ             465000000               // center frequency
#define SX1278_SF               SX1278_LORA_SF_8        // spreading factor
#define SX1278_CR               SX1278_LORA_CR_4_8      // coding rade
#define SX1278_PWR              SX1278_POWER_20DBM      // tx power
#define SX1278_BW               SX1278_LORA_BW_500KHZ   // bandwidth
#define SX1278_CRC_EN           SX1278_LORA_CRC_EN      // disable CRC
#define SX1278_PACKET_LEN       SX1278_MAX_PACKET       // packet length
#define SX1278_PR_LEN           10                      // preamble length


uint8_t radio_init(void);
uint8_t radio_tx_test_transmit(uint32_t sys_tick_ms);
void radio_tx_idle_task(uint32_t sys_tick_ms);
void radio_rx_idle_task(uint32_t sys_tick_ms);

void radio_tx_set_frequency(uint64_t freq);
void radio_tx_set_sf(uint8_t data_in);
void radio_tx_set_cr(uint8_t data_in);
void radio_tx_set_crc(uint8_t crc);
void radio_tx_set_bw(uint16_t data_in);

uint64_t radio_tx_get_frequency(void);
uint8_t radio_tx_get_sf(void);
uint8_t radio_tx_get_cr(void);
uint8_t radio_tx_get_crc(void);
uint8_t radio_tx_get_bw(void);

void radio_tx_test_enable(void);
void radio_tx_test_disable(void);

uint8_t radio_tx_custom_msg(uint8_t* msg, uint8_t len);

#endif // _RADIO_TX_H_