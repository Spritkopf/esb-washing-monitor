#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdint.h>

enum
{
    ESB_CFG_SET_THRESHOLDS   = 0x81,  /*!< Set ADC threshold values */
    ESB_CFG_GET_THRESHOLDS   = 0x82,  /*!< Set ADC threshold values */
    ESB_CFG_SET_CENTRAL_ADDR = 0x83,  /*!< Set ESB Pipeline Address to send notifications to */
    ESB_CFG_SAVE             = 0x84   /*!< Store current configuration in persistent memory */
} esb_cmd_id_config;

typedef struct {
    uint8_t esb_central_addr[5];    /*!< ESB pipeline address of central device, which receives notification from this application */
    int16_t adc_threshold_l;        /*!< ADC threshold to switch from 1 to 0 (hysteresis) */
    int16_t adc_threshold_h;        /*!< ADC threshold to switch from 0 to 1 (hysteresis) */
    uint32_t crc32;                 /*!< CRC checksum */
} app_config_t;

extern app_config_t g_app_config;

/*!
 * \brief Initialize Configuration
 * \details Load configuration from persistent memory, if available
 * \retval 0   OK 
 * \retval -1  mem init error
 * \retval -2 if no configuration was found (defaults applied)
 * \retval -3 Error registering app config ESB commands (no space in command table? check ::ESB_COMMANDS_NUM_APP_TABLES)
 */
int8_t app_config_init(void);

/*!
 * \brief Write current configuration to persistent memory if it has changed
 * \returns 0 if OK, -1 if error writing data
 */
int8_t app_config_save(void);

#endif