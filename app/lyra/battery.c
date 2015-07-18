/** Measures remaining capacity of battery and exposes it as a BLE service.
 *  Modified from nrf ble_sdk_app_hrs example.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <nordic_common.h>
#include <nrf.h>
#include <app_error.h>
#include <nrf_gpio.h>
#include <nrf51_bitfields.h>
#include <softdevice_handler.h>
#include <app_util.h>

#include <ble_ss.h>
#include <ble_common.h>
#include "battery.h"

ble_ss_t battery_ss;

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200                                      /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION         3                                         /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS       270                                       /**< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that is connected in series with the voltage supply. This is the voltage drop when the forward current is 1mA. Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com. */

/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 255) * ADC_PRE_SCALING_COMPENSATION)

/**@brief Function for handling the ADC interrupt.
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
    if (NRF_ADC->EVENTS_END != 0)
    {
        uint8_t     adc_result;
        uint16_t    mv; // battery level in mv
        uint8_t     percent; // battery level in percentage
        uint32_t    ble_msg = 0;
        uint32_t    err;

        NRF_ADC->EVENTS_END     = 0;
        adc_result              = NRF_ADC->RESULT;
        NRF_ADC->TASKS_STOP     = 1;

        mv = ADC_RESULT_IN_MILLI_VOLTS(adc_result) + DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        percent = battery_level_in_percent(mv);

        printf("%s: Battery Level Percentage: %d, level: %dmv\n", __FUNCTION__,
                (int) percent, (int) mv);
        ble_msg = percent | (mv << 16);
        err = ble_ss_sensor_value_update(&battery_ss, (uint8_t *)&ble_msg, sizeof(ble_msg));
        APP_ERROR_CHECK(err);
    }
}

void battery_measure_start(void)
{
    uint32_t err_code;

    // Configure ADC
    NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk;
    NRF_ADC->CONFIG     = (ADC_CONFIG_RES_8bit                        << ADC_CONFIG_RES_Pos)     |
                          (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                          (ADC_CONFIG_REFSEL_VBG                      << ADC_CONFIG_REFSEL_Pos)  |
                          (ADC_CONFIG_PSEL_Disabled                   << ADC_CONFIG_PSEL_Pos)    |
                          (ADC_CONFIG_EXTREFSEL_None                  << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

    // Enable ADC interrupt
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START = 1;
}

void battery_service_init(void)
{
    uint32_t err_code;
    ble_uuid_t ble_service_uuid;
    ble_uuid_t ble_char_uuid;
    ble_ss_init_t bat_ss_param;

    // Initialize lyra_bs Service.
    memset(&bat_ss_param, 0, sizeof(bat_ss_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bat_ss_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bat_ss_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bat_ss_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bat_ss_param.sensor_value_report_read_perm);

    bat_ss_param.evt_write_handler      = NULL;
    bat_ss_param.evt_auth_write_handler = NULL;
    bat_ss_param.evt_auth_read_handler  = NULL;
    bat_ss_param.support_notification   = true;
    bat_ss_param.p_report_ref           = NULL;
    bat_ss_param.initial_value          = 50;

    BLE_UUID_ASSIGN_TYPE_STD(ble_service_uuid, BLE_UUID_BATTERY_SERVICE);
    BLE_UUID_ASSIGN_TYPE_STD(ble_char_uuid, BLE_UUID_BATTERY_LEVEL_STATE_CHAR);
    err_code = ble_ss_init(&battery_ss, &ble_service_uuid, &ble_char_uuid, &bat_ss_param);
    APP_ERROR_CHECK(err_code);
}