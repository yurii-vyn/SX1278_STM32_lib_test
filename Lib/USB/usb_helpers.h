#ifndef _USB_HELPERS_H_
#define _USB_HELPERS_H_

#include "main.h"



void cdc_init(void);
void cdc_msg_print(const char * format, ... );
uint8_t cdc_check_rx_msg(void);
uint32_t cdc_get_rx_len(void);
void cdc_clear_rx_msg(void);

void USB_CDC_RxHandler(uint8_t* Buf, uint32_t Len);


#endif // _USB_HELPERS_H_