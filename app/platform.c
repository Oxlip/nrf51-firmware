/* Platform code common to all the devices.
 */

#include <boards.h>
#include <softdevice_handler.h>
#include <app_timer.h>
#include <app_gpiote.h>

#include <board_conf.h>
#include <board_export.h>
#include "simple_uart.h"
#include "boards.h"

#include "platform.h"
#include "app_button.h"

static uint32_t hz_count;
static uint32_t m_current_time;
static app_timer_id_t frequency_check_timer_id;  /**< Polling timer id. */

/* BLE init routines from ble_common.c */
void ble_init(void);
void ble_late_init(void);

static void
frequency_check_handler (void * p_context)
{
    printf("1: Timer got called: %s %s\n", __DATE__, __TIME__);
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(ADVERTISING_LED_PIN_NO);
    nrf_gpio_cfg_output(CONNECTED_LED_PIN_NO);
    nrf_gpio_cfg_output(ASSERT_LED_PIN_NO);
    nrf_gpio_cfg_output(AURA_TOUCH_LED);

    nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);
    nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);
    nrf_gpio_pin_clear(ASSERT_LED_PIN_NO);
    nrf_gpio_pin_clear(AURA_TOUCH_LED);
}

/* Start of changes sripk */
#define PWM_OUTPUT_PIN_NUMBER (LED_0)  /**< Pin number for PWM output. */

#define TIMER2_PRESCALER             (9UL)                                  /**< Timer 0 prescaler */
#define TIMER2_CLOCK                 (SystemCoreClock >> TIMER2_PRESCALER)  /**< Timer clock frequency */
#define MS_TO_TIMER2_TICKS(ms)       ((1000000UL * ms) / (TIMER2_CLOCK))    /**< Converts milliseconds to timer ticks */

#define MAX_SAMPLE_LEVELS (256UL)  /**< Maximum number of sample levels. */
#define TIMER_PRESCALERS  6U       /**< Prescaler setting for timer. */

/** @brief Function for getting the next sample.
 *  @return sample_value computed sample.
 */
static __INLINE uint32_t next_sample_get(void)
{
    static uint32_t sample_value = 8;
    
    // Read button input.
    sample_value = (~(nrf_gpio_port_read(NRF_GPIO_PORT_SELECT_PORT0)) & 0x000000FFUL);
  
    // This is to avoid having two CC events happen at the same time,
    // CC1 will always create an event on 0 so CC0 and CC2 should not.
    if (sample_value == 0) 
    {
        sample_value = 8;
    }

    return (uint32_t)sample_value;
}


void TIMER0_IRQHandler(void)
{
    printf("Timer0 IRQ\n");
}

void TIMER1_IRQHandler(void)
{
    printf("Timer1 IRQ\n");
}

/** @brief Function for handling timer 2 peripheral interrupts.
 */
void TIMER2_IRQHandler(void)
{
    static int sample = 10;

    if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && \
        ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0))
    {
        printf("Timer2 IRQ handler:sample: %d %s\n", sample, __TIME__);

        NRF_TIMER2->EVENTS_COMPARE[0] = 0;
        NRF_TIMER2->CC[0]            += MS_TO_TIMER2_TICKS((1000 * 1 / 60));
        sample--;
        if (sample < 0) {
            NRF_TIMER2->INTENSET = 0;
            NRF_TIMER2->TASKS_STOP        = 1;                      // Stop timer, if it was running
            NRF_TIMER2->TASKS_CLEAR       = 1;
            NRF_TIMER2->TASKS_START       = 0; // Start clocks
        }

        if ((NRF_TIMER2->EVENTS_COMPARE[1] != 0) && 
                ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
        {
        }
    }
}



/** @brief Function for initializing the Timer 2 peripheral.
 */
static void timer2_init(void)
{
    printf("1: Timer 2 init start\n");
    // Start 16 MHz crystal oscillator .
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
        //Do nothing.
    }

    // Clears the timer, sets it to 0.
    NRF_TIMER2->TASKS_STOP  = 1;
    NRF_TIMER2->TASKS_CLEAR  = 1;
    NRF_TIMER2->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;                      // clean up possible old events
    NRF_TIMER2->EVENTS_COMPARE[1] = 0;
    NRF_TIMER2->EVENTS_COMPARE[2] = 0;
    NRF_TIMER2->EVENTS_COMPARE[3] = 0;
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_24Bit;  
    NRF_TIMER2->PRESCALER = TIMER_PRESCALERS + 3;

    // Load the initial values to TIMER2 CC registers.
    NRF_TIMER2->CC[0] = MS_TO_TIMER2_TICKS((1000 * 1000 / 60));
    NRF_TIMER2->CC[1] = 0;

    // CC2 will be set on the first CC1 interrupt.
    NRF_TIMER2->CC[2] = 0;

    // Enable interrupt for COMPARE[0]
    // Interrupt setup.
    NRF_TIMER2->INTENSET    = (1UL << TIMER_INTENSET_COMPARE0_Pos);

    m_current_time        = 0;

    NRF_POWER->TASKS_CONSTLAT = 1;

    // Enable interrupt on Timer 2.
    NVIC_EnableIRQ(TIMER2_IRQn);
    __enable_irq();

}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
#if 0
    uint32_t err_code;
#endif

    printf("1: %s: APP_TIMER_INIT & Returning\n", __FUNCTION__);
#if 1
    // Initialize timer module, making it use the scheduler
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS,
            APP_TIMER_OP_QUEUE_SIZE, false);
#endif

#if 0
    // Create polling timer.
    err_code = app_timer_create(&frequency_check_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                frequency_check_handler);
    APP_ERROR_CHECK(err_code);
#endif

    if (1) 
    timer2_init();
}

/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    static int led = 0;
    if (button_action == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case AURA_TOUCH_BUTTON:
                /* Do the actual button press handling here. */
                /* For now just turn on an LED */
                if (led) {
                    nrf_gpio_pin_clear(AURA_TOUCH_LED);
                    led = 0;
                } else {
                    nrf_gpio_pin_set(AURA_TOUCH_LED);
                    led = 1;
                }
                break;

            default:
                APP_ERROR_HANDLER(pin_no);
        }
    }
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
#if 1
    return;
#endif
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {AURA_TOUCH_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_event_handler},
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function to start timers.
 */
static void timers_start(void)
{
    uint32_t err_code;

    printf("1: %s: \n", __FUNCTION__);
#if 1
    // Create polling timer.

    if (1) {
        printf("Creating timer for frequency check\n");
    err_code = app_timer_create(&frequency_check_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                frequency_check_handler);
    APP_ERROR_CHECK(err_code);
    }
#endif
#define AURA_ONE_SEC_TIMER APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)  

    if (1) {
    printf("Starting timer for frequency check\n");
    err_code = app_timer_start(frequency_check_timer_id, AURA_ONE_SEC_TIMER,
                               NULL);
    if (err_code != NRF_SUCCESS)
    {
        // The impact in app_button of the app_timer queue running full is losing a button press.
        // The current implementation ensures that the system will continue working as normal. 
    }
    }

#if 1
    printf("1: Starting Timer2\n");
    // Start the timer.
    NRF_TIMER2->TASKS_START = 1;
#endif

}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Implement _write() std library function.
 *        This is needed for printf().
 */
int _write(int fd, const char * str, int len) __attribute__ ((used));
int _write(int fd, const char * str, int len)
{
#ifdef DEBUG
    for (int i = 0; i < len; i++)
    {
        simple_uart_put(str[i]);
    }
#endif
    return len;
}


/**@brief Implement puts() std library function.
 *        This is needed for printf()(which calls puts() if no argument is passed).
 */
int puts(const char *str)
{
#ifdef DEBUG
    return _write(0, str, __builtin_strlen(str));
#else
    return 0;
#endif
}

int fputc(int ch, FILE * p_file) 
{
    simple_uart_put((uint8_t)ch);
    return 0;
}

/**@brief Initialize debug functionality.
 */
static void debug_init(void)
{
#ifdef DEBUG
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, false);
    printf("1: Firmware Date: %s %s\n", __DATE__, __TIME__);
#endif
}


/**@brief Function for application main entry.
 */
int main(void)
{
    hz_count = 0;
    debug_init();

    // Initialize different SOC parts.
    timers_init();
    gpiote_init();
    leds_init();
#if 1
    printf("buttons init");
    buttons_init();
#endif

#if 1
    ble_init();
    scheduler_init();
    ble_late_init();
#endif

    printf("1: calling timer start\n");
    // Start execution
    timers_start();

    // Enter main loop
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}
