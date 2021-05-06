#include <string.h>
#include <stddef.h>
#include "app_config.h"
#include "persistent_mem.h"
#include "crc32.h"
#include <common/commands/esb_commands.h>
#define APP_CONFIG_MEM_OFFSET   0   /* config structure beginning of persistent memory */

app_config_t g_app_config = {
    .esb_central_addr = {0x12,0x12,0x12,0x12,0x12},
    .adc_threshold_l = 5000,
    .adc_threshold_h = 6000,
    .crc32 = 0
};

void esb_cmd_fct_cfg_set_threshold(const esb_protocol_message_t* message, esb_protocol_message_t* answer)
{
    answer->error = ESB_PROT_REPLY_ERR_OK;

    g_app_config.adc_threshold_l = *(int16_t*)&(message->payload[0]);
    g_app_config.adc_threshold_h = *(int16_t*)&(message->payload[0+sizeof(int16_t)]);
   
    return;
}

void esb_cmd_fct_cfg_get_threshold(const esb_protocol_message_t* message, esb_protocol_message_t* answer)
{
    answer->error = ESB_PROT_REPLY_ERR_OK;
    answer->payload_len = 2*sizeof(int16_t);
    *(int16_t*)&(answer->payload[0]) = g_app_config.adc_threshold_l;
    *(int16_t*)&(answer->payload[0+sizeof(int16_t)]) = g_app_config.adc_threshold_h;
    
    return;
}

void esb_cmd_fct_cfg_set_central_addr(const esb_protocol_message_t* message, esb_protocol_message_t* answer)
{
    answer->error = ESB_PROT_REPLY_ERR_OK;
    memcpy(g_app_config.esb_central_addr, message->payload, message->payload_len);
   
    return;
}

void esb_cmd_fct_cfg_save(const esb_protocol_message_t* message, esb_protocol_message_t* answer)
{
    answer->error = ESB_PROT_REPLY_ERR_OK;
    if(app_config_save() != 0){
        answer->error = ESB_PROT_REPLY_ERR_API;
    }
   
    return;
}

#define APP_CONFIG_COMMANDS     4
static esb_cmd_table_item_t esb_cmd_table_config[APP_CONFIG_COMMANDS+1] = 
{
    /* COMMAND_ID              PAYLOAD_SIZE       FUNCTION_POINTER*/
    {ESB_CFG_SET_THRESHOLDS,   2*sizeof(int16_t), esb_cmd_fct_cfg_set_threshold},
    {ESB_CFG_GET_THRESHOLDS,   0,                 esb_cmd_fct_cfg_get_threshold},
    {ESB_CFG_SET_CENTRAL_ADDR, 5,                 esb_cmd_fct_cfg_set_central_addr},
    {ESB_CFG_SAVE,             0,                 esb_cmd_fct_cfg_save},
    
    /* last entry must be NULL-terminator */
    {0,                       0,                 NULL}
};

int8_t app_config_init(void)
{
    int8_t result = 0;
    app_config_t temp_config;
    result = persistent_mem_init();
    if(result != 0){
        return (-1);
    }
    result = persistent_mem_read(APP_CONFIG_MEM_OFFSET, sizeof(app_config_t), (void*)&temp_config);
    if(result != 0){
        return (-1);
    }

    /* CRC-check */
    uint32_t temp_crc32 = crc32_compute((uint8_t*)&temp_config, offsetof(app_config_t, crc32), NULL);
    if(temp_crc32 == temp_config.crc32){
        g_app_config = temp_config;
    }

    /* register config commands in ESB command table */
    esb_protocol_err_t esb_result = esb_commands_register_app_commands(esb_cmd_table_config, APP_CONFIG_COMMANDS);

    if(esb_result != ESB_PROT_ERR_OK){
        return (-3);
    }
    return (0);
}

int8_t app_config_save(void)
{
    uint32_t temp_crc32 = crc32_compute((uint8_t*)&g_app_config, offsetof(app_config_t, crc32), NULL);

    /* only writing to persistent memory when content has changed -> check CRC */
    if(temp_crc32 != g_app_config.crc32){
        g_app_config.crc32 = temp_crc32;

        if(persistent_mem_erase(APP_CONFIG_MEM_OFFSET, 1) != 0){
            return (-1);
        }

        if(persistent_mem_write(APP_CONFIG_MEM_OFFSET, sizeof(app_config_t), (void*)&g_app_config) != 0){
            return (-1);
        }
    }
    return (0);
}
