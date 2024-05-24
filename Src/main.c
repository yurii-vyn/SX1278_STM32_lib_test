#include "main.h"
#include "platform_f103.h"
#include "usb_device.h"
#include "usb_helpers.h"
#include "simple_cli.h"
#include "lib_led.h"
#include "task_vcp_debug.h"
#include "W25Qxx.h"
#include "sandbox.h"
#include "radio_tx.h"


volatile uint32_t timer_global = 0;



int main(void)
{
  platform_init();
  MX_USB_DEVICE_Init();
  led_init();

#ifdef USE_SPI_FLASH
  W25Q_Reset();
#endif

  // HAL_Delay(2000);

  radio_init();

  while (1)
  {
    timer_global = HAL_GetTick();
    
    scli_idle_task();
    task_vcp_test_send(timer_global);
    led_idle_task(timer_global);

#ifdef RADIO_MODE_TX
    radio_tx_idle_task(timer_global);
#endif

#ifdef RADIO_MODE_RX
    radio_rx_idle_task(timer_global);
#endif
  }
}
