#include <stddef.h>

#include "app_esb_cmd_def.h"
#include "adc.h"

enum
{
    ESB_CMD_GET_ADC     = 0x40,   /* Get ADC value */
} esb_cmd_id_app;


/* Get the last measured ADC value
 * payload length: 0
 * payload: None
 * answer length: 2 (sizeof(int16_t))
 * answer payload: (int16_t)ADC value
 * answer error: ESB_PROT_ERR_OK if OK
 */
void app_esb_cmd_fct_get_adc_value(const esb_protocol_message_t* message, esb_protocol_message_t* answer)
{
    int16_t adc_value;
    answer->error = ESB_PROT_REPLY_ERR_OK;

    adc_get_last_value(&adc_value);
    answer->payload_len = sizeof(int16_t);
    *(int16_t*)&(answer->payload[0]) = adc_value;
    
    return;
}


#define APP_ESB_CMD_NUM   1
/*!
 * \brief Command table
 */
esb_cmd_table_item_t app_esb_cmd_table[APP_ESB_CMD_NUM+1] = 
{
    /* COMMAND_ID                      PAYLOAD_SIZE   FUNCTION_POINTER*/
    {ESB_CMD_GET_ADC,                  0,             app_esb_cmd_fct_get_adc_value},
    
    /* last entry must be NULL-terminator */
    {0,                 0,                    NULL}
};



esb_cmd_table_item_t* app_get_esb_cmd_table(uint32_t *num_entries)
{
    *num_entries = (uint32_t)APP_ESB_CMD_NUM;
    return (app_esb_cmd_table);
}