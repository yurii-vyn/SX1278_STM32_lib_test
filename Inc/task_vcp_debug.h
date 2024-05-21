#ifndef _TASK_VCP_DEBUG_H_
#define _TASK_VCP_DEBUG_H_

#include "main.h"

#define PERIOD_VCP_TASK_MS    1000


void task_vcp_test_send(uint32_t sys_tick_ms);
void vcp_task_enable(void);
void vcp_task_diasble(void);


#endif // _TASK_VCP_DEBUG_H_