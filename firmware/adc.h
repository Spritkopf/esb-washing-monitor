#ifndef _ADC_H
#define _ADC_H


#include <stdint.h>

#define ADC_SAMPLE_NUM  1

/**
 * \brief adc data ready callback
 *
 * \param data[in]  buffer containing the ADC data of the current conversion
 */
typedef void (* adc_data_ready_callback_t)(int16_t data[ADC_SAMPLE_NUM]);

/*!
 * \brief Initialize ADC
 * \details ADC is set up for continuous measurements every N milliseconds, triggered by
 *          a timer via PPI
 * \param interval_ms[in]   Interval of the ADC conversions in msec (mus be >0)
 * \retval 0    OK
 * \retval -1   illegal parameter(NULL Pointer)
 */
int8_t adc_init(uint32_t interval_ms);

/*!
 * \brief Get the last measured ADC value
 * \param[out] adc_value   Pointer to buffer for ADC value
 * \retval 0    OK
 * \retval -1   illegal parameter(NULL Pointer)
 */
int8_t adc_get_last_value(int16_t* adc_value);

/*!
 * \brief Start the continuous conversion
 * \param data_ready_callback[in]   callback function to call when converted data is ready
 * \retval 0    OK
 * \retval -1   illegal parameter (NULL Pointer)
 */
int8_t adc_start_conversion(adc_data_ready_callback_t data_ready_callback);

/*!
 * \brief Stop ongoing continuous conversion
 */
void adc_stop_conversion();

#endif