/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup BLE Sensor Service
 * @{
 * @brief Sensor Service module based on Nordic's Battery Service module.
 *
 * @details This module implements the Sensor Service with the arbitary Sensor Value characteristic.
 *          During initialization it adds the Sensor Service and Sensor Value characteristic
 *          to the BLE stack database. 
 *
 *          If specified, the module will support notification of the Sensor Value characteristic
 *          through the ble_ss_sensor_value_update() function.
 *          If an event handler is supplied by the application, the Sensor Service will
 *          generate Sensor Service events to the application.
 *
 * @note The application must propagate BLE stack events to the Sensor Service module by calling
 *       ble_ss_on_ble_evt() from the from the @ref ble_stack_handler callback.
 *
 */

#ifndef BLE_SS_H__
#define BLE_SS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief Sensor Service event type. */
typedef enum
{
    BLE_SS_EVT_NOTIFICATION_ENABLED,                             /**< Sensor value notification enabled event. */
    BLE_SS_EVT_NOTIFICATION_DISABLED                             /**< Sensor value notification disabled event. */
} ble_ss_evt_type_t;

/**@brief Sensor Service event. */
typedef struct
{
    ble_ss_evt_type_t evt_type;                                  /**< Type of event. */
} ble_ss_evt_t;

// Forward declaration of the ble_ss_t type. 
typedef struct ble_ss_s ble_ss_t;

/**@brief Sensor Service event handler type. */
typedef void (*ble_ss_evt_handler_t) (ble_ss_t * p_ss, ble_gatts_evt_write_t * p_evt_write, ble_ss_evt_t * p_evt);

/**@brief Sensor Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_ss_evt_handler_t          evt_handler;                    /**< Event handler to be called for handling events in the Sensor Service. */
    bool                          support_notification;           /**< TRUE if notification of Sensor Level measurement is supported. */
    ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the Sensor Level characteristic */
    uint32_t                      initial_value;                  /**< Initial sensor value */
    ble_srv_cccd_security_mode_t  sensor_value_char_attr_md;      /**< Initial security level for sensor characteristics attribute */
    ble_gap_conn_sec_mode_t       sensor_value_report_read_perm;  /**< Initial security level for sensor report read attribute */
} ble_ss_init_t;

/**@brief Sensor Service structure. This contains various status information for the service. */
typedef struct ble_ss_s
{
    ble_ss_evt_handler_t          evt_handler;                    /**< Event handler to be called for handling events in the Sensor Service. */
    uint16_t                      service_handle;                 /**< Handle of Sensor Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      sensor_value_handles;           /**< Handles related to the Sensor Level characteristic. */
    uint16_t                      report_ref_handle;              /**< Handle of the Report Reference descriptor. */
    uint32_t                      sensor_value_last;              /**< Last Sensor Level measurement passed to the Sensor Service. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                          is_notification_supported;      /**< TRUE if notification of Sensor Level is supported. */
} ble_ss_t;

/**@brief Function for initializing the Sensor Service.
 *
 * @param[out]  p_ss        Sensor Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_ble_uuid  UUID of the service.
 * @param[in]   p_ss_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_ss_init(ble_ss_t * p_ss, ble_uuid_t * p_ble_uuid, const ble_ss_init_t * p_ss_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Sensor Service.
 *
 * @note ble_ss_sensor_value_update() must be called upon reconnection if the
 *       sensor value has changed while the service has been disconnected from a bonded
 *       client.
 *
 * @param[in]   p_ss       Sensor Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_ss_on_ble_evt(ble_ss_t * p_ss, ble_evt_t * p_ble_evt);

/**@brief Function for updating the sensor value.
 *
 * @details The application calls this function after having performed a sensor measurement. If
 *          notification has been enabled, the sensor value characteristic is sent to the client.
 *
 * @note This function must be called upon reconnection if the sensor value has changed
 *       while the service has been disconnected from a bonded client.
 *
 * @param[in]   p_ss          Sensor Service structure.
 * @param[in]   sensor_value  New sensor measurement value (in percent of full capacity).
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_ss_sensor_value_update(ble_ss_t * p_ss, uint32_t sensor_value);

#endif // BLE_SS_H__

/** @} */
