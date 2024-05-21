#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usb_helpers.h"

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
extern USBD_HandleTypeDef hUsbDeviceFS;
uint16_t cdc_tx_len = 0;

uint8_t usb_rx_buffer[APP_RX_DATA_SIZE];
uint8_t cdc_msg_received = 0;
uint32_t cdc_rx_len = 0;


static void _cdc_clear_tx_buffer(void);
static void _cdc_clear_rx_buffer(void);
static void _cdc_transmit(void);


/**
 * ----------------- Public -----------------
*/

void cdc_init(void)
{
  MX_USB_DEVICE_Init();
}

void cdc_msg_print(const char * format, ... )
{
  va_list ap;

  _cdc_clear_tx_buffer();

  va_start(ap, format);
  cdc_tx_len = vsnprintf((char*)UserTxBufferFS, APP_TX_DATA_SIZE, format, ap);
  va_end(ap);

  _cdc_transmit();
}

/**
 * Check if there is already something in RX buffer
*/
uint8_t cdc_check_rx_msg(void)
{
  return cdc_msg_received;
}

uint32_t cdc_get_rx_len(void)
{
  if(cdc_msg_received != 0){
    return cdc_rx_len;
  }

  return 0;
}

void cdc_clear_rx_msg(void)
{
  _cdc_clear_rx_buffer();
  cdc_msg_received = 0;
  cdc_rx_len = 0;
}

void USB_CDC_RxHandler(uint8_t* Buf, uint32_t Len)
{
  memcpy(usb_rx_buffer, Buf, Len);
  cdc_msg_received = 1;
  cdc_rx_len = Len;
}



/**
 * ----------------- Private -----------------
*/

static void _cdc_clear_tx_buffer(void)
{
  memset(UserTxBufferFS, 0x00, APP_TX_DATA_SIZE);
  cdc_tx_len = 0;
}

static void _cdc_clear_rx_buffer(void)
{
  memset(usb_rx_buffer, 0x00, APP_RX_DATA_SIZE);
  cdc_rx_len = 0;
  cdc_msg_received = 0;
}

static void _cdc_transmit(void)
{
  CDC_Transmit_FS(UserTxBufferFS, cdc_tx_len);
}

