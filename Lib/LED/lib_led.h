#ifndef _LIB_LED_H_
#define _LIB_LED_H_

#include "main.h"


#define LED_Pin                     GPIO_PIN_13
#define LED_GPIO_Port               GPIOC

#define LED_BLINK_PERIOD_SHORT_MS   200
#define LED_BLINK_PERIOD_NORMAL_MS  500
#define LED_BLINK_PERIOD_LONG_MS    1000
#define LED_BLINK_PERIOD_MAX_MS     10000

typedef enum {
  LED_ONBOARD, 
  LED_CNT
}led_e;

typedef enum {
  LED_MODE_OFF,
  LED_MODE_ON,
  LED_MODE_BLINK,
  LED_MODE_CNT
}led_mode_t;

typedef struct {
  led_mode_t mode;
  uint32_t timer;
  uint16_t period;
}led_attr_t;



void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);
void led_write_state(led_mode_t mode);
void led_update(uint32_t sys_tick_ms);
void led_idle_task(uint32_t sys_tick_ms);
void led_force_update(void);
void led_set_mode(led_e led, led_mode_t mode);
void led_set_period(led_e led, uint16_t period_ms);
led_mode_t led_get_mode(led_e led);
uint16_t led_get_period(led_e led);

#endif // _LIB_LED_H_