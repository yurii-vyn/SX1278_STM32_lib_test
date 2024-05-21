#include "platform_f103.h"
#include "lib_led.h"



led_attr_t leds[LED_CNT];

static void led_gpio_init(void);

void led_init(void)
{
  led_gpio_init();

  leds[LED_ONBOARD].mode = LED_MODE_BLINK;
  leds[LED_ONBOARD].period = LED_BLINK_PERIOD_NORMAL_MS;
  leds[LED_ONBOARD].timer = 0;
}

void led_on(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void led_off(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void led_toggle(void)
{
  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void led_write_state(led_mode_t mode)
{
  switch (mode)
  {
  case LED_MODE_OFF:
    led_off();
    break;
  case LED_MODE_ON:
    led_on();
    break;
  case LED_MODE_BLINK:
    led_toggle();
    break;
  
  default:
    break;
  }
}

/**
 * Idle task 
 */
void led_idle_task(uint32_t sys_tick_ms)
{
  led_update(sys_tick_ms);
}


void led_update(uint32_t sys_tick_ms)
{
  for(uint8_t i = 0; i < LED_CNT; i++){
    if(sys_tick_ms >= leds[i].timer){
      leds[i].timer = sys_tick_ms + leds[i].period;
      led_write_state(leds[i].mode);
    }
  }
}

void led_force_update(void)
{
  uint32_t sys_tick_f_ms = 0;
  sys_tick_f_ms = HAL_GetTick();

  for(uint8_t i = 0; i < LED_CNT; i++){
    led_write_state(leds[i].mode);
    leds[i].timer = sys_tick_f_ms + leds[i].period;
  }
}

void led_set_mode(led_e led, led_mode_t mode)
{
  if((led < LED_CNT) && (mode < LED_MODE_CNT)){
    leds[led].mode = mode;
  }
}

void led_set_period(led_e led, uint16_t period_ms)
{
  if((led < LED_CNT) && (period_ms <= LED_BLINK_PERIOD_MAX_MS)){
    leds[led].period = period_ms;
  }
}

led_mode_t led_get_mode(led_e led)
{
  if(led < LED_CNT){
    return leds[led].mode;
  }

  return LED_MODE_CNT;
}

uint16_t led_get_period(led_e led)
{
  if(led < LED_CNT){
    return leds[led].period;
  }

  return 0;
}

static void led_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}