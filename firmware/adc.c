
#include "adc.h"
#include "nrfx_saadc.h"
#include "nrfx_ppi.h"
#include "nrfx_timer.h"

static nrfx_timer_t m_timer = NRFX_TIMER_INSTANCE(0);
static nrf_ppi_channel_t m_ppi_channel;
static nrf_saadc_value_t adc_buffer[ADC_SAMPLE_NUM];

static adc_data_ready_callback_t m_callback = NULL;

/* timeout handler is empty, timer is only used to trigger ADC conversion via PPI */
void timer_handler(nrf_timer_event_t event_type, void *p_context){}

void saadc_callback(nrfx_saadc_evt_t const * p_event)
{
    if (p_event->type == NRFX_SAADC_EVT_DONE)
    {
        nrfx_saadc_buffer_convert(p_event->data.done.p_buffer, ADC_SAMPLE_NUM);

        if(m_callback != NULL)
        {
            m_callback((int16_t*)p_event->data.done.p_buffer);
        }

    }
}

int8_t adc_init(uint32_t interval_ms)
{
    if(interval_ms == 0)
    {
        return (-1);
    }

    /* Initialize SAADC peripheral */
    nrfx_saadc_config_t config = NRFX_SAADC_DEFAULT_CONFIG;
    config.resolution = NRF_SAADC_RESOLUTION_14BIT;
    /**< Channel instance. Default configuration used (ANALOG input 2 -> Pin P0.04). */
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
    channel_config.gain = SAADC_CH_CONFIG_GAIN_Gain1_6;
    nrfx_saadc_init(&config, saadc_callback);

    nrfx_saadc_channel_init(0, &channel_config);
    nrfx_saadc_buffer_convert(adc_buffer, ADC_SAMPLE_NUM);

    /* Initialize timer */
    nrfx_timer_config_t timer_cfg = {
        .frequency = NRF_TIMER_FREQ_1MHz,
        .mode = NRF_TIMER_MODE_TIMER,
        .bit_width = NRF_TIMER_BIT_WIDTH_32,
        .interrupt_priority = 6,
        .p_context = NULL
    };
    nrfx_timer_init(&m_timer, &timer_cfg, timer_handler);
    nrfx_timer_extended_compare(&m_timer, 
                                NRF_TIMER_CC_CHANNEL0, 
                                nrf_timer_ms_to_ticks(interval_ms, timer_cfg.frequency), 
                                NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
    nrfx_timer_enable(&m_timer);

    /* Initialize PPI
       The timer compare event will be connected to the saadc conversion task, triggering an
       ADC conversion on every timer compare event */
    uint32_t timer_task_addr_sample = nrfx_timer_compare_event_address_get(&m_timer, NRF_TIMER_CC_CHANNEL0);
    uint32_t adc_task_addr_sample = nrfx_saadc_sample_task_get();
    nrfx_ppi_channel_alloc(&m_ppi_channel);

    nrfx_ppi_channel_assign(m_ppi_channel, timer_task_addr_sample, adc_task_addr_sample);

    return (0);
}

int8_t adc_get_last_value(int16_t* adc_value)
{
    if(adc_value == NULL){
        return (-1);
    }
    *adc_value = adc_buffer[0];

    return (0);
    
}

int8_t adc_start_conversion(adc_data_ready_callback_t data_ready_callback)
{
    if(data_ready_callback == NULL)
    {
        return (-1);
    }

    m_callback = data_ready_callback;

    nrfx_ppi_channel_enable(m_ppi_channel);
    
    return (0);
}

void adc_stop_conversion()
{
    nrfx_ppi_channel_disable(m_ppi_channel);
}