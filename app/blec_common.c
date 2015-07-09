/**
 * Common BLE central mode code.
 */
#include <boards.h>
#include <softdevice_handler.h>
#include <app_timer.h>
#include <ble_types.h>
#include <ble_advdata.h>
#include <ble_dis.h>
#include <ble_conn_params.h>
#include <ble_gap.h>
#include <pstorage.h>
#include <device_manager_s130.h>
#include <ble_db_discovery_s130.h>

#include <board_conf.h>
#include <board_export.h>
#include <ble_uuids.h>

#include "platform.h"
#include "ble_common.h"
#include "blec_common.h"

#include <ble_hci.h>
#include <ble_dfu.h>
#include <dfu_app_handler.h>

#define LOG printf
#define SCAN_INTERVAL              0x00A0                             /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                0x0050                             /**< Determines scan window in units of 0.625 millisecond. */
#define MIN_CONNECTION_INTERVAL    MSEC_TO_UNITS(50, UNIT_1_25_MS)    /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL    MSEC_TO_UNITS(100, UNIT_1_25_MS)   /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY              0                                  /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_UUID                0xa000                             /**< Target device name that application is looking for. */
#define MAX_PEER_COUNT             DEVICE_MANAGER_MAX_CONNECTIONS     /**< Maximum number of peer's application intends to manage. */
#define UUID16_SIZE                2                                  /**< Size of 16 bit UUID */
#define RSSI_CRITERIA              -90                                /**< Minimum RSSI value for peer peripheral. */
/**@breif Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST,SRC)                                                                  \
        do                                                                                       \
        {                                                                                        \
            (*(DST)) = (SRC)[1];                                                                 \
            (*(DST)) <<= 8;                                                                      \
            (*(DST)) |= (SRC)[0];                                                                \
        } while(0)

 /**@brief Variable length data encapsulation in terms of length and pointer to data */
typedef struct
{
    uint8_t     * p_data;                                         /**< Pointer to data. */
    uint16_t      data_len;                                       /**< Length of data. */
} data_t;

typedef enum
{
    BLE_NO_SCAN,                                                  /**< No advertising running. */
    BLE_WHITELIST_SCAN,                                           /**< Advertising with whitelist. */
    BLE_FAST_SCAN,                                                /**< Fast advertising running. */
} ble_advertising_mode_t;
static uint16_t                     m_peripheral_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_db_discovery_t           m_ble_db_discovery;                  /**< Structure used to identify the DB Discovery module. */
static dm_application_instance_t    m_dm_app_id;                         /**< Application identifier. */
static dm_handle_t                  m_dm_device_handle;                  /**< Device Identifier identifier. */
static uint8_t                      m_peer_count = 0;                    /**< Number of peer's connected. */
static uint8_t                      m_scan_mode;                         /**< Scan mode used by application. */
static bool                         m_memory_access_in_progress = false; /**< Flag to keep track of ongoing operations on persistent memory. */

/**< Scan parameters requested for scanning and connection. */
ble_gap_scan_params_t m_scan_param;

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

/**@breif Function to start scanning.
 */
void blec_scan_start(void)
{
    ble_gap_whitelist_t   whitelist;
    ble_gap_addr_t        * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t         * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    uint32_t              err_code;
    uint32_t              count;

    printf("BLE Scan Start::\n");

    // Verify if there is any flash access pending, if yes delay starting scanning until 
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);
    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    // Initialize whitelist parameters.
    whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    whitelist.irk_count  = 0;
    whitelist.pp_addrs   = p_whitelist_addr;
    whitelist.pp_irks    = p_whitelist_irk;

    // Request creating of whitelist.
    err_code = dm_whitelist_create(&m_dm_app_id, &whitelist);
    APP_ERROR_CHECK(err_code);

    if (((whitelist.addr_count == 0) && (whitelist.irk_count == 0)) ||
         (m_scan_mode != BLE_WHITELIST_SCAN))
    {
        // No devices in whitelist, hence non selective performed.
        m_scan_param.active       = 0;            // Active scanning set.
        m_scan_param.selective    = 0;            // Selective scanning not set.
        m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
        m_scan_param.window       = SCAN_WINDOW;  // Scan window.
        m_scan_param.p_whitelist  = NULL;         // No whitelist provided.
        m_scan_param.timeout      = 0x0000;       // No timeout.
    }
    else
    {
        // Selective scanning based on whitelist first.
        m_scan_param.active       = 0;            // Active scanning set.
        m_scan_param.selective    = 1;            // Selective scanning not set.
        m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
        m_scan_param.window       = SCAN_WINDOW;  // Scan window.
        m_scan_param.p_whitelist  = &whitelist;   // Provide whitelist.
        m_scan_param.timeout      = 0x001E;       // 30 seconds timeout.

        // Set whitelist scanning state.
        m_scan_mode = BLE_WHITELIST_SCAN;
    }

    err_code = sd_ble_gap_scan_start(&m_scan_param);
    APP_ERROR_CHECK(err_code);
}

void blec_sys_event_handler(uint32_t sys_evt)
{
    switch(sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:
            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                blec_scan_start();
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Callback handling device manager events.
 *
 * @details This function is called to notify the application of device manager events.
 *
 * @param[in]   p_handle      Device Manager Handle. For link related events, this parameter
 *                            identifies the peer.
 * @param[in]   p_event       Pointer to the device manager event.
 * @param[in]   event_status  Status of the event.
 */
static api_result_t device_manager_event_handler(const dm_handle_t    * p_handle,
                                                 const dm_event_t     * p_event,
                                                 const api_result_t     event_result)
{
    uint32_t err_code;

    switch (p_event->event_id)
    {
        case DM_EVT_CONNECTION:
        {
            LOG("[APPL]: >> DM_EVT_CONNECTION\n");
            ble_gap_addr_t * peer_addr;
            peer_addr = &p_event->event_param.p_gap_param->params.connected.peer_addr;
            LOG("[APPL]:[%02X %02X %02X %02X %02X %02X]: Connection Established\n",
                                peer_addr->addr[0], peer_addr->addr[1], peer_addr->addr[2],
                                peer_addr->addr[3], peer_addr->addr[4], peer_addr->addr[5]);
            nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);
            m_dm_device_handle = (*p_handle);

            /*
             * TODO: Add a check if we want to discover for this peer (ble_gap_addr_t)
             * If not contiue with other connections and gracefully terminate
             * this connection.
             */
            // Discover peer's services.
            err_code = ble_db_discovery_start(&m_ble_db_discovery,
                                              p_event->event_param.p_gap_param->conn_handle);
            APP_ERROR_CHECK(err_code);

            m_peer_count++;

            if (m_peer_count < MAX_PEER_COUNT)
            {
                blec_scan_start();
            }
            LOG("[APPL]: << DM_EVT_CONNECTION\n");
            break;
        }

        case DM_EVT_DISCONNECTION:
        {
            LOG("[APPL]: >> DM_EVT_DISCONNECTION\n");
            memset(&m_ble_db_discovery, 0 , sizeof (m_ble_db_discovery));

            nrf_gpio_pin_set(CONNECTED_LED_PIN_NO);

            if (m_peer_count == MAX_PEER_COUNT)
            {
                blec_scan_start();
            }
            m_peer_count--;
            LOG("[APPL]: << DM_EVT_DISCONNECTION\n");
            break;
        }

        case DM_EVT_SECURITY_SETUP:
        {
            LOG("[APPL]:[0x%02X] >> DM_EVT_SECURITY_SETUP\n", p_handle->connection_id);
            // Slave securtiy request received from peer, if from a non bonded device, 
            // initiate security setup, else, wait for encryption to complete.
            err_code = dm_security_setup_req(&m_dm_device_handle);
            APP_ERROR_CHECK(err_code);
            LOG("[APPL]:[0x%02X] << DM_EVT_SECURITY_SETUP\n", p_handle->connection_id);
            break;
        }

        case DM_EVT_SECURITY_SETUP_COMPLETE:
        {
            LOG("[APPL]: >> DM_EVT_SECURITY_SETUP_COMPLETE\n");
            break;
        }

        case DM_EVT_LINK_SECURED:
            LOG("[APPL]: >> DM_LINK_SECURED_IND\n");
            LOG("[APPL]: << DM_LINK_SECURED_IND\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_LOADED:
            LOG("[APPL]: >> DM_EVT_LINK_SECURED\n");
            APP_ERROR_CHECK(event_result);
            LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_LOADED\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_STORED:
            LOG("[APPL]: >> DM_EVT_DEVICE_CONTEXT_STORED\n");
            APP_ERROR_CHECK(event_result);
            LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_STORED\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_DELETED:
            LOG("[APPL]: >> DM_EVT_DEVICE_CONTEXT_DELETED\n");
            APP_ERROR_CHECK(event_result);
            LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_DELETED\n");
            break;

        default:
            LOG("[APPL]: unknown device manager event\n");
            break;
    }

    return NRF_SUCCESS;
}

/**@brief Function for initializing the Device Manager.
 *
 * @details Device manager is initialized here.
 */
void device_manager_init(void)
{
    dm_application_param_t param;
    dm_init_param_t        init_param;

    uint32_t               err_code;

    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    init_param.clear_persistent_data = true;
    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&param.sec_param, 0, sizeof (ble_gap_sec_params_t));

    // Event handler to be registered with the module.
    param.evt_handler            = device_manager_event_handler;

    // Service or protocol context for device manager to load, store and apply on behalf of application.
    // Here set to client as application is a GATT client.
    param.service_type           = DM_PROTOCOL_CNTXT_GATT_CLI_ID;

    // Secuirty parameters to be used for security procedures.
    param.sec_param.bond         = SEC_PARAM_BOND;
    param.sec_param.mitm         = SEC_PARAM_MITM;
    param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    param.sec_param.oob          = SEC_PARAM_OOB;
    param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    param.sec_param.kdist_periph.enc = 1;
    param.sec_param.kdist_periph.id  = 1;

    err_code = dm_register(&m_dm_app_id, &param);
    APP_ERROR_CHECK(err_code);
}


void blec_gap_event_timeout(const ble_gap_evt_t *p_gap_evt, uint8_t timeout_src)
{
    if(timeout_src == BLE_GAP_TIMEOUT_SRC_SCAN)
    {
        LOG("[GAP event]: Scan timed out.\n");
        m_scan_mode = BLE_FAST_SCAN;
        blec_scan_start();
    }
    else if (timeout_src == BLE_GAP_TIMEOUT_SRC_CONN)
    {
        LOG("[GAP event]: Connection Request timed out.\n");
    }
    LOG("BLE GAP event\n");
}

/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type = p_data[index+1];

        if (field_type == type)
        {
            p_typedata->p_data = &p_data[index+2];
            p_typedata->data_len = field_length-1;
            return NRF_SUCCESS;
        }
        index += field_length+1;
    }
    return NRF_ERROR_NOT_FOUND;
}

/**@brief Function to handle BLE advertisements(in central mode).
 */
void blec_gap_event_advertisement_report(ble_evt_t *p_ble_evt)
{
    data_t adv_data;
    data_t type_data;
    uint32_t err_code;
    const ble_gap_evt_t *p_gap_evt = &p_ble_evt->evt.gap_evt;

    // Initialize advertisement report for parsing.
    adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
    adv_data.data_len = p_gap_evt->params.adv_report.dlen;

    err_code = adv_report_parse(BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE,
                                &adv_data,
                                &type_data);
    if (err_code != NRF_SUCCESS)
    {
        // Compare short local name in case complete name does not match.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                    &adv_data,
                                    &type_data);
    }

    // Verify if short or complete name matches target.
    if (err_code == NRF_SUCCESS)
    {
        uint16_t extracted_uuid;
        LOG("[BLE ADV] : ");

        // UUIDs found, look for matching UUID
        /* Need to compare 128 bits of UUID */
        for (uint32_t u_index = 0; u_index < (type_data.data_len/UUID16_SIZE); u_index++)
        {
            UUID16_EXTRACT(&extracted_uuid, &type_data.p_data[u_index * UUID16_SIZE]);

            LOG("UUID: %x-, TARGET UUID: %x\n", (unsigned int)
                    extracted_uuid, (unsigned int) TARGET_UUID);

            if(extracted_uuid == TARGET_UUID)
            {
                // Stop scanning and initiate connection if the rssi value is larger than RSSI_CRITERIA
                if(p_ble_evt->evt.gap_evt.params.adv_report.rssi >= RSSI_CRITERIA)
                {
                    // Stop scanning.
                    err_code = sd_ble_gap_scan_stop();
                    if (err_code != NRF_SUCCESS)
                    {
                        LOG("[ADV_REP]: Scan stop failed, reason %ld\n", err_code);
                    }

                    err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,
                                                  &m_scan_param, &m_connection_param);

                    if (err_code != NRF_SUCCESS)
                    {
                        LOG("[ADV_REP]: Connection Request Failed, reason %ld\n", err_code);
                    }
                }
                return;
            }
        }
        LOG("\n ");
    }
}


/**@brief Function for initializing security parameters.
 */
static void scan_params_init(void)
{
    m_scan_param.active       = 0;            // Active scanning set.
    m_scan_param.selective    = 0;            // Selective scanning not set.
    m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
    m_scan_param.window       = SCAN_WINDOW;  // Scan window.
    m_scan_param.p_whitelist  = NULL;         // No whitelist provided.
    m_scan_param.timeout      = 0x001E;       // 30 seconds timeout.
}

/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init();
    APP_ERROR_CHECK(err_code);
}

void blec_init()
{
    scan_params_init();
    device_manager_init();
    db_discovery_init();
}

void blec_on_ble_evt (ble_evt_t *p_ble_evt)
{
    dm_ble_evt_handler(&m_peripheral_conn_handle, p_ble_evt); 
    ble_db_discovery_on_ble_evt(&m_peripheral_conn_handle, &m_ble_db_discovery, p_ble_evt);
}
