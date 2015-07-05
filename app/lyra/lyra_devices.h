#ifndef __LYRA_DEVICES_H__
#define __LYRA_DEVICES_H__

#define LYRA_MAX_BUTTONS 3
#define LYRA_MAX_ACTIONS 3
#define LYRA_MAX_ACTION_COUNT ((LYRA_MAX_BUTTONS) * (LYRA_MAX_ACTIONS))

typedef struct lyra_button_action_ {
    uint8_t address[6]; /* address of the device */
    uint8_t action;
    uint8_t value;
} lyra_button_action_t;

typedef struct lyra_button_char_event_ {
    uint8_t action; /* Add/remove */
    uint8_t button_number;
    uint8_t action_index;
    uint8_t address[6];
    uint8_t device_type;
    uint8_t value;
    uint8_t padding;
} lyra_button_char_event_t;

int
read_si7013(float *temperature, int32_t *humidity);

float
lux_to_pct(float lux);

float
get_ambient_lux();

void
dm_store_write_data_evt(uint8_t *data, uint8_t len);
void
lyra_handle_button_event (uint8_t pin_no, uint8_t action);

#endif /* __LYRA_DEVICES_H__ */
