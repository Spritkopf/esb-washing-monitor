
#include "nrf_esb.h"

#include <stdint.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_error.h"

#include "nrf_queue.h"

#include "adc.h"
#include "app_config.h"
#include "app_esb_cmd_def.h"
#include <common/protocol/esb_protocol.h>
#include <binary-sensor/binary_sensor.h>

#define WASHING_MACHINE_FINISHED_LED_PROPERTY_ID    0

static const uint8_t esb_listener_address[5] = {111,111,111,111,1}; /* temporary, for tests. */

/*!
 * \brief Data ready callback
 * \details Contains the main logic of the washing monitor application. 
 * The ADC is connected to a photoresistor and periodically delivers data 
 * which represents the brightness of the light source (LED) at the photoresistor. 
 * When the ADC value exceeds a certain threshold, the LED is considered on. Vice versa,
 * when falling below a threshold, the LED is considered off. To avoid oscillating a hysteresis is
 * implemented with an upper and a lower threshold. The threshold values are configurable via
 * ESB configuration commands
 */ 
static void adc_data_ready_callback(int16_t data[ADC_SAMPLE_NUM])
{
    
    if(data[0] > g_app_config.adc_threshold_h){
        binary_sensor_set_channel(WASHING_MACHINE_FINISHED_LED_PROPERTY_ID, CHAN_VAL_TRUE);
    }else if(data[0] < g_app_config.adc_threshold_l){
        binary_sensor_set_channel(WASHING_MACHINE_FINISHED_LED_PROPERTY_ID, CHAN_VAL_FALSE);
    }

    /* publish the state: send a notification to the central device, if the state has changed */
    binary_sensor_publish();
}

void clocks_start( void )
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}

void power_manage( void )
{
    // WFE - SEV - WFE sequence to wait until a radio event require further actions.
    __SEV();
    __WFE();
    __WFE();
}

int main(void)
{
    clocks_start();

    esb_protocol_init(esb_listener_address);

    uint32_t num_app_cmds = 0;

    /* register application ESB commands */
    esb_cmd_table_item_t* app_cmd_table = app_get_esb_cmd_table(&num_app_cmds);
    esb_commands_register_app_commands(app_cmd_table, num_app_cmds);

    app_config_init();
    
    binary_sensor_init(esb_listener_address);
    binary_sensor_set_central_address(g_app_config.esb_central_addr);
    
    adc_init(3000);
    adc_start_conversion(adc_data_ready_callback);

    while (true)
    {
        esb_protocol_process();
        power_manage();
    }
}
