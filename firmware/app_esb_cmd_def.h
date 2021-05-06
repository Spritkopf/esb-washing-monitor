
#ifndef APP_ESB_CMD_DEF_H_
#define APP_ESB_CMD_DEF_H_

#include <common/commands/esb_commands.h>

/*! 
 * \brief get pointer to application command table 
 * \param[out] num_entries number of entries in the command table
 */
esb_cmd_table_item_t* app_get_esb_cmd_table(uint32_t *num_entries);

#endif /* APP_ESB_CMD_DEF_H_ */
