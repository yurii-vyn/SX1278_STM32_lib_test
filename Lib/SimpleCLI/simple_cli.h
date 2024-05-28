#ifndef _SIMPLE_CLI_H_
#define _SIMPLE_CLI_H_

#include "main.h"
#include "lib_led.h"




#define SCLI_PARAM_NAME_LEN     3                         // parameter name length - if you cange this value, you should change all names in .c
#define SCLI_HEADER_LEN         (SCLI_PARAM_NAME_LEN+3)
#define SCLI_SERVICE_LEN        (SCLI_HEADER_LEN+1)

#define SCLI_STX1               'Q'                       // message should start with "QM" 
#define SCLI_STX2               'M'
#define SCLI_ETX                '\n'                      // and end with LF

#define SCLI_SET                '<'                       // for example, send "QM<SPF=8" (with auto LF on) to set SF to 8
#define SCLI_GET                '>'                       // send "QM>SPF" to get current SF value in return

#define SCLI_CMD_LED_ON         LED_MODE_ON
#define SCLI_CMD_LED_OFF        LED_MODE_OFF
#define SCLI_CMD_LED_TOGGLE     10
#define SCLI_CMD_LED_BLINK      LED_MODE_BLINK


typedef enum {
  SCLI_RX_IDLE,
  SCLI_RX_STX1,
  SCLI_RX_STX2,
  SCLI_RX_CMD,
  SCLI_RX_PARAM,
  SCLI_RX_ETX,
  SCLI_RX_DATA,
  SCLI_RX_COMPLETE,
  SCLI_RX_ERROR
}scli_state_t;

typedef enum {
  SCLI_ERR_CLEAR,
  SCLI_ERR_RX,
  SCLI_ERR_STX1,
  SCLI_ERR_STX2,
  SCLI_ERR_CMD,
  SCLI_ERR_PARAM,
  SCLI_ERR_ETX,
  SCLI_ERR_DATA
}scli_error_t;

typedef enum {
  SCLI_PRES_ERROR,
  SCLI_PRES_OK
}scli_pres_t;

enum {
  SCLI_POS_STX1,
  SCLI_POS_STX2,
  SCLI_POS_CMD,
  SCLI_POS_PARAM,
  SCLI_POS_DATA
};

typedef enum param_id{
  SCLI_PARAM_ID_LED_MODE, 
  SCLI_PARAM_ID_LED_PERIOD,
  SCLI_PARAM_ID_TIME,
  SCLI_PARAM_ID_VERSION,
  SCLI_PARAM_ID_DEBUG_ENABLE,
  SCLI_PARAM_ID_FLASH_ID,
  SCLI_PARAM_ID_FREQ,
  SCLI_PARAM_ID_SFACTOR,
  SCLI_PARAM_ID_CODRATE,
  SCLI_PARAM_ID_CRC,
  SCLI_PARAM_ID_BANDWIDTH,
  SCLI_PARAM_ID_ENTTX,
  SCLI_PARAM_ID_CUSTOM_TX,
  SCLI_PARAM_ID_FHSS_PERIOD,
  SCLI_PARAM_ID_CNT
} param_id_t;

typedef struct scli_param{
  param_id_t id;
  uint8_t name[SCLI_PARAM_NAME_LEN];
} scli_param_t;


/**
 * Main CLI task
 * 
 * Should be always called from idle in order to not block anything else.
*/
void scli_idle_task(void);

#endif // _SIMPLE_CLI_H_