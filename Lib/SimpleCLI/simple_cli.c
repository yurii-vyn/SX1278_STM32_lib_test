#include "platform_f103.h"
#include "simple_cli.h"
#include "usbd_cdc_if.h"
#include "usb_helpers.h"
#include "lib_led.h"
#include "W25Qxx.h"
#include "SX1278.h"
#include "radio_tx.h"



const scli_param_t scli_param_list[SCLI_PARAM_ID_CNT] = {
  {.id = SCLI_PARAM_ID_LED_MODE,      .name = "LMD"},
  {.id = SCLI_PARAM_ID_LED_PERIOD,    .name = "LPR"},
  {.id = SCLI_PARAM_ID_TIME,          .name = "TIM"},
  {.id = SCLI_PARAM_ID_VERSION,       .name = "VER"},
  {.id = SCLI_PARAM_ID_DEBUG_ENABLE,  .name = "DBE"},
  {.id = SCLI_PARAM_ID_FLASH_ID,      .name = "FID"},
  {.id = SCLI_PARAM_ID_FREQ,          .name = "FRQ"},
  {.id = SCLI_PARAM_ID_SFACTOR,       .name = "SPF"},
  {.id = SCLI_PARAM_ID_CODRATE,       .name = "CRT"},
  {.id = SCLI_PARAM_ID_CRC,           .name = "CRC"},
  {.id = SCLI_PARAM_ID_BANDWIDTH,     .name = "BDW"},
  {.id = SCLI_PARAM_ID_ENTTX,         .name = "ETT"},
  {.id = SCLI_PARAM_ID_CUSTOM_TX,     .name = "CTX"},
};

extern uint8_t usb_rx_buffer[APP_RX_DATA_SIZE];
scli_state_t scli_rx_sate = SCLI_RX_IDLE;
scli_error_t scli_error = SCLI_ERR_CLEAR;
scli_pres_t slii_parse_result = SCLI_PRES_ERROR;



static scli_pres_t scli_parse_msg(uint8_t* msg, uint32_t len);
static param_id_t scli_get_param_id(uint8_t* msg_param);
static scli_pres_t scli_process_data(uint8_t* msg, param_id_t param_id, uint16_t data_len);
static void scli_set_rx_error(uint8_t error_code);
static void scli_print_version(void);
static void scli_handle_error(void);

static void msg_led_mode_handler(uint8_t cmd, int data);
static void msg_led_period_handler(uint8_t cmd, int data);
static void msg_time_handler(uint8_t cmd, int data);
static void msg_version_handler(uint8_t cmd, int data);
static void msg_debug_handler(uint8_t cmd, int data);
static void msg_freq_handler(uint8_t cmd, int data);
static void msg_sf_handler(uint8_t cmd, int data);
static void msg_cr_handler(uint8_t cmd, int data);
static void msg_crc_handler(uint8_t cmd, int data);
static void msg_bw_handler(uint8_t cmd, int data);
static void msg_entx_handler(uint8_t cmd, int data);
static void msg_custom_tx_handler(uint8_t cmd, uint8_t* data, uint8_t len);

#ifdef USE_SPI_FLASH
static void msg_flash_handler(uint8_t cmd, int data);
#endif


void scli_idle_task(void)
{
  if(cdc_check_rx_msg() > 0){
    slii_parse_result = scli_parse_msg(usb_rx_buffer, cdc_get_rx_len());
    cdc_clear_rx_msg();

    if(slii_parse_result == SCLI_PRES_ERROR){
      scli_handle_error();
    }
  }
}

static scli_pres_t scli_parse_msg(uint8_t* msg, uint32_t len)
{
  scli_pres_t result = SCLI_PRES_ERROR;
  uint16_t rx_data_len = 0;
  param_id_t param = 0;

  while((scli_rx_sate != SCLI_RX_COMPLETE) && (scli_rx_sate != SCLI_RX_ERROR))
  {
    switch (scli_rx_sate)
    {
    case SCLI_RX_IDLE:
      if(len > 1){
        scli_rx_sate = SCLI_RX_STX1;
      }else{
        scli_set_rx_error(SCLI_ERR_RX);
      }
      break;
    case SCLI_RX_STX1:
      if(msg[SCLI_POS_STX1] == SCLI_STX1){
        scli_rx_sate = SCLI_RX_STX2;
      }else{
        scli_set_rx_error(SCLI_ERR_STX1);
      }
      break;
    case SCLI_RX_STX2:
      if(msg[SCLI_POS_STX2] == SCLI_STX2){
        scli_rx_sate = SCLI_RX_CMD;
        if((msg[SCLI_POS_CMD] == '\r') || (msg[SCLI_POS_CMD] == '\n') || (msg[SCLI_POS_CMD] == '\0')){
          cdc_msg_print("OK\r\n");
          return SCLI_PRES_OK;
        }
      }else{
        scli_set_rx_error(SCLI_ERR_STX2);
      }
      break;
    case SCLI_RX_CMD:
      if((msg[SCLI_POS_CMD] == SCLI_SET) || (msg[SCLI_POS_CMD] == SCLI_GET)){
        scli_rx_sate = SCLI_RX_PARAM;
      }else{
        scli_set_rx_error(SCLI_ERR_CMD);
      }
      break;
    case SCLI_RX_PARAM:
      if(msg[SCLI_POS_PARAM] != 0){
        param = scli_get_param_id(msg+SCLI_POS_PARAM);
        if(param != SCLI_PARAM_ID_CNT){
          scli_rx_sate = SCLI_RX_ETX;
        }else{
          scli_set_rx_error(SCLI_ERR_PARAM);
        }
      }else{
        scli_set_rx_error(SCLI_ERR_PARAM);
      }
      break;
    case SCLI_RX_ETX:
      if(msg[(len-1)] == SCLI_ETX){
        scli_rx_sate = SCLI_RX_DATA;
        rx_data_len = len - SCLI_SERVICE_LEN;
      }else{
        scli_set_rx_error(SCLI_ERR_ETX);
      }
      break;
    case SCLI_RX_DATA:
      if(rx_data_len != 0){
        if(scli_process_data(msg, param, rx_data_len) != SCLI_PRES_OK){
          scli_set_rx_error(SCLI_ERR_DATA);
        }else{
          scli_rx_sate = SCLI_RX_COMPLETE;
        }      
      }else{
        scli_set_rx_error(SCLI_ERR_DATA);
      }
      break;
    default:
      break;
    }
  }

  if(scli_rx_sate == SCLI_RX_COMPLETE){
    result = SCLI_PRES_OK;
  }

  scli_rx_sate = SCLI_RX_IDLE;

  return result;
}

static void scli_handle_error(void)
{
  switch (scli_error)
  {
  case SCLI_ERR_CLEAR:
    cdc_msg_print("No errors\r\n");
    break;
  case SCLI_ERR_RX:
    cdc_msg_print("SCLI_ERR_RX\r\n");
    break;
  case SCLI_ERR_STX1:
    cdc_msg_print("SCLI_ERR_STX1\r\n");
    break;
  case SCLI_ERR_STX2:
    cdc_msg_print("SCLI_ERR_STX2\r\n");
    break;
  case SCLI_ERR_CMD:
    cdc_msg_print("SCLI_ERR_CMD\r\n");
    break;
  case SCLI_ERR_PARAM:
    cdc_msg_print("SCLI_ERR_PARAM\r\n");
    break;
  case SCLI_ERR_ETX:
    cdc_msg_print("SCLI_ERR_ETX\r\n");
    break;
  case SCLI_ERR_DATA:
    cdc_msg_print("SCLI_ERR_DATA\r\n");
    break;
  default:
    break;
  }
}

static param_id_t scli_get_param_id(uint8_t* msg_param)
{
  uint8_t str[SCLI_PARAM_NAME_LEN];
  memcpy(str, msg_param, SCLI_PARAM_NAME_LEN);

  for(uint8_t i = 0; i < SCLI_PARAM_NAME_LEN; i++){
    if(str[i] == 0){
      return SCLI_PARAM_ID_CNT; // return error if string is incomplete or empty
    }
  }

  uint8_t match = 0;

  for(uint8_t i = 0; i < SCLI_PARAM_ID_CNT; i++){
    for(uint8_t j = 0; j < SCLI_PARAM_NAME_LEN; j++){
      if(str[j] != scli_param_list[i].name[j]){
        match = 0;
        break;
      }else{
        match++;
      }
    }
    if(match == SCLI_PARAM_NAME_LEN){
      return (param_id_t)i;
    }
  }

  return SCLI_PARAM_ID_CNT;
}

static scli_pres_t scli_process_data(uint8_t* msg, param_id_t param_id, uint16_t data_len)
{
  // UNUSED(data_len);
  scli_pres_t result = SCLI_PRES_OK;
  int data = 0;

  data = atoi((const char*)(msg+SCLI_HEADER_LEN+1));

  switch(param_id)
  {
  case SCLI_PARAM_ID_LED_MODE:
    msg_led_mode_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_LED_PERIOD:
    msg_led_period_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_TIME:
    msg_time_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_VERSION:
    msg_version_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_DEBUG_ENABLE:
    msg_debug_handler(msg[SCLI_POS_CMD], data);
    break;
#ifdef USE_SPI_FLASH
  case SCLI_PARAM_ID_FLASH_ID:
    msg_flash_handler(msg[SCLI_POS_CMD], data);
    break;
#endif
  case SCLI_PARAM_ID_FREQ:
    msg_freq_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_SFACTOR:
    msg_sf_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_CODRATE:
    msg_cr_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_CRC:
    msg_crc_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_BANDWIDTH:
    msg_bw_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_ENTTX:
    msg_entx_handler(msg[SCLI_POS_CMD], data);
    break;
  case SCLI_PARAM_ID_CUSTOM_TX:
    if((data_len < UINT8_MAX) && (data_len < SX1278_MAX_PACKET)){
      uint8_t data_len_tmp = 0;
      data_len_tmp += data_len;
      msg_custom_tx_handler(msg[SCLI_POS_CMD], (uint8_t*)(msg+SCLI_HEADER_LEN+1), data_len_tmp);
    }else{
      cdc_msg_print("Error: Message is too long\r\n");
    }
    break;
  default:
    result = SCLI_PRES_ERROR;
    cdc_msg_print("Error: Unknown parameter\r\n");
    break;
  }

  return result;
}

static void scli_set_rx_error(uint8_t error_code)
{
  scli_rx_sate = SCLI_RX_ERROR;
  scli_error = error_code;
}

static void scli_print_version(void)
{
  cdc_msg_print("SCLI v.1.0: %s, %s\r\n", __DATE__, __TIME__);
}

static void msg_led_mode_handler(uint8_t cmd, int data)
{
  led_mode_t mode = 0;

  if(cmd == SCLI_SET){
    led_set_mode(LED_ONBOARD, (led_mode_t)data);
    led_force_update();
    mode = led_get_mode(LED_ONBOARD);
    cdc_msg_print("Mode: %d\r\n", (uint8_t)mode);
  }else if(cmd == SCLI_GET){
    mode = led_get_mode(LED_ONBOARD);
    cdc_msg_print("Current mode: %d\r\n", (uint8_t)mode);
  }
}

static void msg_led_period_handler(uint8_t cmd, int data)
{
  uint16_t period = 0;

  if(cmd == SCLI_SET){
    led_set_period(LED_ONBOARD, (uint16_t)data);
    led_force_update();
    period = led_get_period(LED_ONBOARD);
    cdc_msg_print("Period: %d\r\n", period);
  }else if(cmd == SCLI_GET){
    period = led_get_period(LED_ONBOARD);
    cdc_msg_print("Current period: %d\r\n", period);
  }
}

static void msg_time_handler(uint8_t cmd, int data)
{
  UNUSED(data);
  if(cmd == SCLI_GET){
    // UNUSED
  }
}

static void msg_version_handler(uint8_t cmd, int data)
{
  UNUSED(data);
  if(cmd == SCLI_GET){
    scli_print_version();
  }
}

static void msg_debug_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    if(data == 1){
      vcp_task_enable();
      cdc_msg_print("Debug messages enabled\r\n");
    }else if(data == 0){
      vcp_task_diasble();
      cdc_msg_print("Debug messages disabled\r\n");
    }else{
      cdc_msg_print("WDYM?\r\n");
    }
  }
}

#ifdef USE_SPI_FLASH
static void msg_flash_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_GET){
    uint32_t W25Q_chip_id = 0;
    uint16_t W25Q_mid = 0;
    uint8_t W25Q_mid_b[2];
    uint8_t flash_data[8];
    
    W25Q_mid = W25Q_Read_MID();
    W25Q_mid_b[0] = (W25Q_mid & 0xFF00) >> 8;
    W25Q_mid_b[1] = W25Q_mid & 0xFF;
    W25Q_chip_id = W25Q_Read_JEDEC_ID();

    cdc_msg_print("MID: 0x%02X\r\nDevID: 0x%02X\r\nJEDEC ID: %lu\r\n", W25Q_mid_b[0], W25Q_mid_b[1], W25Q_chip_id);
    HAL_Delay(100);
    W25Q_Read(0, 0, 8, flash_data);
    cdc_msg_print("Header:\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n0x%02X\r\n", flash_data[0], flash_data[1], flash_data[2], flash_data[3], flash_data[4], flash_data[5], flash_data[6], flash_data[7]);

    if(data == 1){
      flash_data[0] = 'H';
      flash_data[1] = 'e';
      flash_data[2] = 'l';
      flash_data[3] = 'l';
      flash_data[4] = 'o';
      flash_data[5] = '=';
      flash_data[6] = ')';
      W25Q_Write_Chunk(0, flash_data, 8);
    }
  }
}
#endif

static void msg_freq_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    radio_tx_set_frequency((uint64_t)data);
    cdc_msg_print("Frequency set: %luMHz\r\n", (uint32_t)(data/1000000));
  }else if( cmd == SCLI_GET){
    cdc_msg_print("SX1278 frequency: %luMHz\r\n", (uint32_t)(radio_tx_get_frequency()/1000000));
  }
}

static void msg_sf_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    radio_tx_set_sf((uint8_t)data);
    cdc_msg_print("SF set: %d\r\n", (uint8_t)data);
  }else if( cmd == SCLI_GET){
    cdc_msg_print("SX1278 SF: %d\r\n", radio_tx_get_sf());
  }
}

static void msg_cr_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    radio_tx_set_cr((uint8_t)data);
    cdc_msg_print("CR set: %d\r\n", (uint8_t)data);
  }else if( cmd == SCLI_GET){
    cdc_msg_print("SX1278 CR: %d\r\n", radio_tx_get_cr());
  }
}

static void msg_crc_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    radio_tx_set_crc((uint8_t)data);
    cdc_msg_print("CRC set: %d\r\n", (uint8_t)data);
  }else if( cmd == SCLI_GET){
    cdc_msg_print("SX1278 CRC: %d\r\n", radio_tx_get_crc());
  }
}

static void msg_bw_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    radio_tx_set_bw((uint8_t)data);
    cdc_msg_print("BW set: %d\r\n", (uint8_t)data);
  }else if( cmd == SCLI_GET){
    cdc_msg_print("SX1278 BW: %d\r\n", radio_tx_get_bw());
  }
}

static void msg_entx_handler(uint8_t cmd, int data)
{
  if(cmd == SCLI_SET){
    if(data == 1){
      radio_tx_test_enable();
    }else if(data == 0){
      radio_tx_test_disable();
    }
  }else if(cmd == SCLI_GET){

  }
}

static void msg_custom_tx_handler(uint8_t cmd, uint8_t* data, uint8_t len)
{
  if(cmd == SCLI_SET){
    radio_tx_custom_msg(data, len);
  }else if(cmd == SCLI_GET){

  }
}