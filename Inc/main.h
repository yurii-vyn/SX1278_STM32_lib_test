#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "platform_f103.h"


void vcp_task_enable(void);
void vcp_task_diasble(void);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
