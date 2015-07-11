#include <string.h>
#include <stdio.h>
#include <nrf_gpio.h>
#include <ble.h>
#include <ble_hci.h>
#include <app_util.h>
#include <app_error.h>

#include <ble_uuids.h>
#include <ble_ss.h>
#include <ble_common.h>
#include <app_timer.h>
#include <board_conf.h>

#define SCAN_INTERVAL              0x00A0                             /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                0x0050                             /**< Determines scan window in units of 0.625 millisecond. */
#define MIN_CONNECTION_INTERVAL    MSEC_TO_UNITS(50, UNIT_1_25_MS)    /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL    MSEC_TO_UNITS(100, UNIT_1_25_MS)   /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY              0                                  /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Determines supervision time-out in units of 10 millisecond. */


/**< Scan parameters requested for scanning and connection. */
static ble_gap_scan_params_t m_scan_param = {
    .active       = 0,            // Active scanning set.
    .selective    = 0,            // Selective scanning not set.
    .interval     = SCAN_INTERVAL,// Scan interval.
    .window       = SCAN_WINDOW,  // Scan window.
    .p_whitelist  = NULL,         // No whitelist provided.
    .timeout      = 0x001E,       // 30 seconds timeout.
};

/**
 * @brief Connection parameters requested for connection.
 */
const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    0,                                   // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

typedef struct peer_info_ {
    ble_gap_addr_t peer_addr;
    uint8_t buffer[4];                      /* buffer to write to peer device */
    uint8_t buffer_len;                     /* length of the buffer */

    uint8_t curr_state;                     /* current state of the state machine */

    /* start & end handle for services discovered */
    ble_gattc_handle_range_t srv_handle_range;

    uint16_t conn_handle;                   /**< Handle of the current connection. */
    uint8_t  curr_srv_ind;                  /**< Index of the current service being discovered. This is intended for internal use during service discovery.*/
    uint8_t  curr_char_ind;                 /**< Index of the current characteristic being discovered. This is intended for internal use during service discovery.*/
} peer_info_t;

#define MAX_CONNECTIONS     4
peer_info_t peer_array[MAX_CONNECTIONS];    /* Number of peers that are connected */

#define SRV_DISC_START_HANDLE  0x000b       /**< The start handle value used during service discovery. */

/**
 * Compare two BLE addresses.
 *  Returns True if the addresses are same.
 */
static inline int
compare_ble_addr(uint8_t *addr1, uint8_t *addr2)
{
    return memcmp(&addr1, addr2, BLE_GAP_ADDR_LEN) == 0;
}

/**
 * Helper to print BLE address.
 *  Returns True if the addresses are same.
 */
static inline void
print_ble_addr(uint8_t *addr1)
{
    int i;
    for(i = 0; i < BLE_GAP_ADDR_LEN; i++) {
        printf("%0x:", addr1[i]);
    }
}


static uint8_t invalid_peer_addr[BLE_GAP_ADDR_LEN] = {-1, -1, -1, -1, -1, -1};
/*
 * Finds a free peer_info structure.
 * If nothing is free then NULL is returned.
 */
static peer_info_t *
get_free_peer_info()
{
    int i;
    for(i = 0; i < MAX_CONNECTIONS; i++) {
        if (compare_ble_addr(peer_array[i].peer_addr.addr, invalid_peer_addr) == 0) {
            return &peer_array[i];
        }
    }
    return NULL;
}

/*
 * Finds peer_info for a given BLE address in the current connections.
 * If not found returns NULL.
 */
static peer_info_t *
find_peer_by_ble_addr(uint8_t *addr)
{
    int i;
    for(i = 0; i < MAX_CONNECTIONS; i++) {
        if (compare_ble_addr(peer_array[i].peer_addr.addr, addr) == 0) {
            return &peer_array[i];
        }
    }
    return NULL;
}

/*
 * Finds peer_info for a given BLE connection handle..
 * If not found returns NULL.
 */
static peer_info_t *
find_peer_by_conn_handle(uint16_t conn_handle)
{
    int i;
    for(i = 0; i < MAX_CONNECTIONS; i++) {
        if (peer_array[i].conn_handle == conn_handle) {
            return &peer_array[i];
        }
    }
    return NULL;
}

/*
 * This below function should be generic:
 * to take params, peer_addr, buffer & buffer_len
 */
void
write_to_peer_device (ble_gap_addr_t peer_addr, uint8_t *value, uint8_t len)
{
    uint32_t err_code;
    peer_info_t *peer_info;

    printf("%s: Sending value %#x to peer: ", __FUNCTION__, (int) *value);
    for (int i; i < 6; i++) printf("%02x:", peer_addr.addr[i]);
    printf("\n");

    peer_info = get_free_peer_info();
    if (peer_info == NULL) {
        printf("All connections are active.\n");
        goto retry;
    }

    /* Store the value in a location for this peer */
    peer_info->peer_addr = peer_addr;
    memcpy(peer_info->buffer, value, len);
    peer_info->buffer_len = len;

    peer_info->curr_state = BLE_GAP_EVT_DISCONNECTED;

    err_code = sd_ble_gap_connect(&peer_info->peer_addr,
                                  &m_scan_param, &m_connection_param);
    if (err_code == NRF_SUCCESS) {
        return;
    }

    printf("Connection Request Failed, reason %ld\n", err_code);

retry:
    return;
    //TODO - Add retry logic through timer and blink LED.
}

/*
 * Start primary service discovery.
 */
static void
start_ps_discovery(peer_info_t *peer_info)
{
    uint32_t  err_code;

    printf("Starting primary services discovery\n");
    //TODO - start from 0 and move till we find ours
    err_code = sd_ble_gattc_primary_services_discover(peer_info->conn_handle,
                                                      SRV_DISC_START_HANDLE, NULL);
    APP_ERROR_CHECK(err_code);
}

/*
 * Start Characteristics discovery.
 */
static void
start_char_discovery(peer_info_t *peer_info)
{
    uint32_t  err_code;
    ble_gattc_handle_range_t range;

    range.start_handle = peer_info->srv_handle_range.start_handle;
    range.end_handle = peer_info->srv_handle_range.end_handle;

    printf("Starting char discovery\n");
    err_code = sd_ble_gattc_characteristics_discover(peer_info->conn_handle, &range);
    APP_ERROR_CHECK(err_code);
    // TODO - add retry logic(and timeout after sometime)
}

static void
start_write(peer_info_t *peer_info, uint16_t attribute_handle)
{
    uint32_t  err_code;
    ble_gattc_write_params_t write_params;

    /* send (write) dimmer / button value to peer */
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.handle   = attribute_handle;
    write_params.p_value  = (uint8_t *)&peer_info->buffer;
    write_params.len      = peer_info->buffer_len;
    write_params.offset   = 0;
    /* Write the value to dimmer service */
    err_code = sd_ble_gattc_write(peer_info->conn_handle, &write_params);
    APP_ERROR_CHECK(err_code);
}


/*
 * State machine change to connected state.
 */
static void inline
sm_handle_connected(ble_gap_addr_t peer_addr, uint16_t conn_handle)
{
    peer_info_t *peer_info;

    peer_info = find_peer_by_ble_addr(peer_addr.addr);
    if (peer_info == NULL) {
        print_ble_addr(peer_addr.addr);
        printf(" - not in state machine\n");
        return;
    }

    peer_info->curr_state = BLE_GAP_EVT_CONNECTED;
    peer_info->conn_handle = conn_handle;

    start_ps_discovery(peer_info);
}

/*
 * State machine change to primarary service discovered..
 */
static void inline
sm_handle_ps_discovered(uint16_t conn_handle, uint16_t gatt_status,
                        ble_gattc_evt_prim_srvc_disc_rsp_t *ps_disc_rsp_evt)
{
    peer_info_t *peer_info;

    printf("Got primary service discovery response: status %d\n", gatt_status);

    peer_info = find_peer_by_conn_handle(conn_handle);
    if (peer_info == NULL) {
        printf("Ignoring primary service discovered for conn %d\n", conn_handle);
        return;
    }

    if (gatt_status != BLE_GATT_STATUS_SUCCESS) {
        printf("%s gatt error: %x\n", __FUNCTION__, gatt_status);
        start_ps_discovery(peer_info);
        return;
    }

    if (peer_info->curr_state != BLE_GAP_EVT_CONNECTED) {
        printf("Invalid state: %d\n", peer_info->curr_state);
        return;
    }
    peer_info->curr_state = BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP;

    // TODO walk through all services to find ours.
    peer_info->srv_handle_range = ps_disc_rsp_evt->services[0].handle_range;
    start_char_discovery(peer_info);
}


/*
 * State machine change to characteristics discovered..
 */
static void inline
sm_handle_char_discovered(uint16_t conn_handle, uint16_t gatt_status,
                          ble_gattc_evt_char_disc_rsp_t *char_disc_rsp_evt)
{
    peer_info_t *peer_info;

    printf("Got char discovery response: status %d\n", gatt_status);

    peer_info = find_peer_by_conn_handle(conn_handle);
    if (peer_info == NULL) {
        printf("Ignoring char discovered event for conn %d\n", conn_handle);
        return;
    }

    if (gatt_status != BLE_GATT_STATUS_SUCCESS) {
        printf("%s gatt error: %x\n", __FUNCTION__, gatt_status);
        start_char_discovery(peer_info);
        return;
    }

    if (peer_info->curr_state != BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP) {
        printf("Invalid state: %d\n", peer_info->curr_state);
        return;
    }


    peer_info->curr_state = BLE_GATTC_EVT_CHAR_DISC_RSP;

    start_write(peer_info, char_disc_rsp_evt->chars[0].handle_value);
}

/*
 * State machine change to write completed.
 */
static void inline
sm_handle_write_response(uint16_t conn_handle, uint16_t gatt_status)
{
    peer_info_t *peer_info;

    printf("Got write response: status %d\n", gatt_status);

    peer_info = find_peer_by_conn_handle(conn_handle);
    if (peer_info == NULL) {
        printf("Ignoring write response event for conn %d\n", conn_handle);
        goto disconnect;
    }

    if (gatt_status != BLE_GATT_STATUS_SUCCESS) {
        printf("%s gatt error: %x\n", __FUNCTION__, gatt_status);
        goto disconnect;
    }

    if (peer_info->curr_state != BLE_GATTC_EVT_CHAR_DISC_RSP) {
        printf("Invalid state: %d\n", peer_info->curr_state);
        //todo - retry
        return;
    }

    peer_info->curr_state = BLE_GATTC_EVT_WRITE_RSP;

disconnect:
    sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
}

/*
 * State machine change to disconnect.
 */
static void inline
sm_handle_disconnected(uint16_t conn_handle)
{
    peer_info_t *peer_info;
    peer_info = find_peer_by_conn_handle(conn_handle);
    if (peer_info == NULL) {
        return;
    }
    peer_info->conn_handle = BLE_CONN_HANDLE_INVALID;
    peer_info->curr_state = BLE_GAP_EVT_DISCONNECTED;
    memcpy(&peer_info->peer_addr.addr, invalid_peer_addr, sizeof(invalid_peer_addr));
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
void ble_peer_on_ble_evt (ble_evt_t * p_ble_evt)
{
    ble_gap_evt_t *p_gap_evt;
    ble_gattc_evt_t *p_gattc_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_gap_evt = &p_ble_evt->evt.gap_evt;
            if (p_gap_evt->params.connected.role != BLE_GAP_ROLE_CENTRAL) {
                return;
            }
            sm_handle_connected(p_gap_evt->params.connected.peer_addr, p_gap_evt->conn_handle);

            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            p_gattc_evt = &p_ble_evt->evt.gattc_evt;
            sm_handle_ps_discovered(p_gattc_evt->conn_handle, p_gattc_evt->gatt_status,
                                    &p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp);
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            p_gattc_evt = &p_ble_evt->evt.gattc_evt;
            sm_handle_char_discovered(p_gattc_evt->conn_handle, p_gattc_evt->gatt_status,
                                      &p_ble_evt->evt.gattc_evt.params.char_disc_rsp);
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            printf("Got desc discovery response: status %d\n",
                   p_ble_evt->evt.gattc_evt.gatt_status);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            sm_handle_write_response(p_gattc_evt->conn_handle, p_gattc_evt->gatt_status);

        case BLE_GAP_EVT_DISCONNECTED:
            sm_handle_disconnected(p_gattc_evt->conn_handle);
            break;

        default:
            printf("Unknown BLE event %#x\n", p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}

