#ifndef __LYRA_H__
#define __LYRA_H__

typedef enum {
    BLE_ACTION_INFO_OP_ADD,
    BLE_ACTION_INFO_OP_REMOVE
} ble_action_info_op_t;

typedef struct {
    uint8_t operation;      /* Add/remove the action info. */
    uint8_t button;         /* Button number. */
    uint8_t sub_index;      /* Action index. */
    uint8_t address[6];     /* Address of the BLE target device. */
    uint8_t device_type;    /* Aura/Libra/Hub etc*/
    uint8_t value;          /* Value to send when button pressed. */
    uint8_t padding;
} ble_action_info_msg_t;

void store_button_action(ble_action_info_msg_t *data);
void write_to_peer_device(ble_gap_addr_t peer_addr, uint8_t *value, uint8_t len);

#endif
