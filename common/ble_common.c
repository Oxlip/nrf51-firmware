/**
 * Common code to initalize BLE stack(softdevice) and handle BLE events.
 */

#include <softdevice_handler_appsh.h>
#include <app_timer.h>
#include <ble_types.h>
#include <ble_advdata.h>
#include <ble_dis.h>
#include <ble_conn_params.h>
#include <ble_gap.h>
#include <pstorage.h>
#define BLE_DFU_APP_SUPPORT
#ifdef BLE_DFU_APP_SUPPORT
#include <ble_dfu.h>
#include <dfu_app_handler.h>
#endif // BLE_DFU_APP_SUPPORT

#include <boards.h>
#include <board_export.h>
#include <ble_uuids.h>

#include "platform.h"
#include "common.h"
#include "ble_common.h"

#include <ble_hci.h>

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

static ble_dfu_t                         m_dfus;                                    /**< Structure used to identify the DFU service. */

#endif // BLE_DFU_APP_SUPPORT


#define LOG printf


/**< UUID type registered with the SDK */
uint8_t oxlip_uuid_type = BLE_UUID_TYPE_UNKNOWN;

/**< Security requirements for this application. */
static ble_gap_sec_params_t m_sec_params;

/**< Handle of the current connection. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    printf("BLE Advertisement event %d\n", ble_adv_evt);
}


static ble_advdata_t advdata;

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    uint32_t      err_code;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for setting the service data in the advertisement packets.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 */
void ble_advertising_service_data_set(ble_advdata_service_data_t *service_data)
{
    uint32_t      err_code;

    if (!service_data) {
        return;
    }

    advdata.p_service_data_array    = service_data;
    advdata.service_data_count      = 1;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing security parameters.
 */
static void sec_params_init(void)
{
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


static void handle_gap_event_timeout(ble_evt_t *p_ble_evt)
{
    const ble_gap_evt_t *p_gap_evt = &p_ble_evt->evt.gap_evt;
    uint8_t timeout_src = p_gap_evt->params.timeout.src;

    printf("BLE_GAP_TIMEOUT src = %d\n", timeout_src);

    if (timeout_src == BLE_GAP_TIMEOUT_SRC_CONN) {
        set_connection_indicator(0);
    } else if (timeout_src == BLE_GAP_TIMEOUT_SRC_ADVERTISING) {
#ifndef USE_CENTRAL_MODE
        ble_advertising_start(BLE_ADV_MODE_FAST);
#endif
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    static ble_gap_evt_auth_status_t m_auth_status __attribute__ ((unused));;
    ble_gap_enc_info_t *             p_enc_info __attribute__ ((unused));;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            set_connection_indicator(1);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            set_connection_indicator(0);
            ble_advertising_start(BLE_ADV_MODE_FAST);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_SUCCESS,
                                                   &m_sec_params,
                                                   //bonding is not supported for now.
                                                   NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            handle_gap_event_timeout(p_ble_evt);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            printf("BLE_GAP_EVT_AUTH_STATUS - Not handled.\n");
            break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            printf("BLE_GAP_EVT_SEC_INFO_REQUEST - Not handled.\n");
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            printf("BLE_GAP_EVT_CONN_PARAM_UPDATE - Not handled.\n");
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            printf("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST - Not handled.\n");
            break;

        default:
            printf("Unknown BLE event %#x\n", p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    device_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
#endif // BLE_DFU_APP_SUPPORT
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
}


/**@brief Initialize Device Information Service.
 *
 * @details Uses macros from board_info.h platform.h to build ble_dis.
 *
 */
static void device_information_service_init(void)
{
    uint32_t       err_code;
    ble_dis_init_t dis_init;
    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.serial_num_str, (char *)"FILL MAC_ID HERE");
    //ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, (char *)DEVICE_HW_REVISION_ID);
    //ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, (char *)DEVICE_FW_REVISION_ID);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

}

/**@brief Registers vendor specific UUID.
 *
 */
static void uuid_init(void)
{
    uint32_t err_code;
    ble_uuid128_t base_uuid = {BLE_UUID_BASE};

    err_code = sd_ble_uuid_vs_add(&base_uuid, &oxlip_uuid_type);
    APP_ERROR_CHECK(err_code);
}

static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    set_advertisement_indicator(0);
}

/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by 
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
static void reset_prepare(void)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_APPSH_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_4000MS_CALIBRATION, true);

    // Enable BLE stack
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = 1;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

#ifdef BLE_DFU_APP_SUPPORT
static void dfu_service_init()
{
    uint32_t err_code;
    ble_dfu_init_t dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.revision      = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare);
    //dfu_app_dm_appl_instance_set(m_app_handle);
}
#endif // BLE_DFU_APP_SUPPORT


/**@brief Function for initializing the BLE.
 *
 */
void ble_init()
{
    ble_stack_init();
}


/**@brief Function for initializing the BLE.
 *
 */
void ble_late_init()
{
    gap_params_init();
    uuid_init();
    services_init();

#ifdef BLE_DFU_APP_SUPPORT
    dfu_service_init();
#endif
    conn_params_init();
    sec_params_init();

    device_information_service_init();
    advertising_init();
    uint32_t err_code;
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}
