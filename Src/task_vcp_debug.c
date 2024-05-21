#include "task_vcp_debug.h"
#include "usb_helpers.h"

volatile uint32_t timer_task_vcp = 0;
uint8_t vcp_task_enabled = 0;

void task_vcp_test_send(uint32_t sys_tick_ms)
{
  if((sys_tick_ms >= timer_task_vcp) && (vcp_task_enabled == 1)){
    timer_task_vcp = sys_tick_ms + PERIOD_VCP_TASK_MS;
    cdc_msg_print("%lu\r\n", (uint32_t)(sys_tick_ms/1000));
  }
}

void vcp_task_enable(void)
{
  vcp_task_enabled = 1;
}

void vcp_task_diasble(void)
{
  vcp_task_enabled = 0;
}