/**
 *
 * Driver for Dimming an light/fan using Triac and Zero crossing interrupt.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>

#include "platform.h"
#include "board_conf.h"
#include "dimmer.h"

typedef struct {
    app_timer_id_t  timer_id;
    uint8_t         enabled;
    uint32_t        ticks;
} dimmer_config_t;

dimmer_config_t dimmer_config[PLATFORM_MAX_TRIACS];

static int dimmer_configured = 0;
static uint32_t dimmer_cb_granularity_ms;
static app_gpiote_user_id_t zc_gpiote_id;

/*
 * \brief Zero Cross timer callback.
 *
 * This callback is invoked when it is time to turn on Triac.
 *
 * \note This wont be called when dimming option is either 0 or 100%.
 *
 * \param rt    Timer.
 * \param ptr   unused.
 */
void dimmer_timer_callback(void* context)
{
    int device = (dimmer_config_t *)context - &dimmer_config[0];
    triac_set(device, TRIAC_OPERATION_ON);
}

/*
 * \brief Zero Cross ISR callback.
 *
 * When the Zero Cross circuit detects AC sine wave's zero cross it generates an
 * interrupt which is handled by an ISR which calls this function.
 * So this function will be called at 50Hz frequency(100 times per second).
 * This function schedules a RT timer, such that it will be fired when
 * the TRIAC has to be turned on.
 *
 */
static void
zc_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
    int i;

    for(i = 0; i < sizeof(dimmer_config) / sizeof(dimmer_config_t); i++)
    {
        /* For Dim percentage of 100 we don't start the timer,
         * for the rest the timer fires at the closest approximation.
         */
        uint32_t result;
        dimmer_config_t *config = &dimmer_config[i];
        if(config->enabled == 1 && config->ticks != 0) {
            triac_set(i, TRIAC_OPERATION_OFF);

            result = app_timer_start(config->timer_id, config->ticks, config);
            if(result != NRF_SUCCESS) {
                printf("Error Setting timer for device %d %#lx\n", i, result);
            }
        }
    }
}

/*
 * \brief Enable dimmer and set the brightness.
 *
 * \param triac         Triac number(0-4).
 * \param brightness    Brightness in percentage(0-100).
 */
void
dimmer_enable(int triac, int brightness)
{
    dimmer_config_t *config = &dimmer_config[triac];
    int darkness = 100 - brightness;

    ble_dimmer_update_value(brightness);

    if (brightness == 0) {
        printf("Turning off\n");
        triac_set(triac, TRIAC_OPERATION_OFF);
        return;
    } else {
        printf("Turning on\n");
        triac_set(triac, TRIAC_OPERATION_ON);
        return;
    }

    if (!config->enabled) {
        dimmer_configured++;
    }

    config->enabled = 1;
    uint32_t timeout = dimmer_cb_granularity_ms * (darkness / 10);
    config->ticks = APP_TIMER_TICKS(timeout, APP_TIMER_PRESCALER);

    /* If this is the first triac that needs to be dimmed,
       enable the zero cross interrupt */
    if (dimmer_configured == 1) {
        app_gpiote_user_enable(zc_gpiote_id);
    }
}

/*
 * \brief Disable dimmer.
 *
 * \param triac     Triac number(0-4).
 */
void
dimmer_disable(int triac)
{
    dimmer_config_t *config = &dimmer_config[triac];

    if (config->enabled == 0) {
        return;
    }
    config->enabled = 0;

    triac_set(triac, TRIAC_OPERATION_ON);

    dimmer_configured--;
    /* If this is the last triac that is being disabled, disable the zero cross interrupt */
    if (dimmer_configured == 0) {
        app_gpiote_user_disable(zc_gpiote_id);
    }
}

/*
 * \brief Initialize the dimmer code.
 */
void
dimmer_init(uint8_t ac_frequency)
{
    /* Can be a macro if zc_frequency is known a-priori, else need
    * to initialize these based on calibration. TODO
    */
    uint8_t  zc_frequency;
    uint32_t zc_interval_ms;

    // Configure zero crossing as sense interrupt.
    nrf_gpio_cfg_sense_input(AURA_ZERO_CROSSING_PIN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
    app_gpiote_user_register(&zc_gpiote_id, 0, 1 << AURA_ZERO_CROSSING_PIN, zc_event_handler);
    app_gpiote_user_enable(zc_gpiote_id);

    for(int i = 0; i < sizeof(dimmer_config) / sizeof(dimmer_config_t); i++)
    {
        uint32_t err_code;
        dimmer_config_t *config;

        config = &dimmer_config[i];
        config->enabled = 0;
        err_code = app_timer_create(&config->timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                    dimmer_timer_callback);
        APP_ERROR_CHECK(err_code);
    }

    /* The number of times ZC interrupt will be called per second.      (50hz)*/
    zc_frequency = 2 * ac_frequency;

    /* Time in milliseconds between Zero cross Interrupts.              (10ms)*/
    zc_interval_ms = 1000 / zc_frequency;

    /* Time in milliseconds when we divide the interval into 10 equal parts. (1ms)*/
    dimmer_cb_granularity_ms = zc_interval_ms / 10;
}

