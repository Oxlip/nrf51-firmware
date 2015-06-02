/*
 * Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**@file
 *
 * @defgroup ble_sdk_srv_dim_c Dimmer Service Client
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    Dimmer Service Client module.
 *
 * @details  This module contains APIs to read and interact with the Dimmer Service of a remote
 *           device.
 *
 * @note     The application must propagate BLE stack events to this module by calling
 *           ble_hrs_c_on_ble_evt().
 *
 */

#ifndef BLE_DIM_C_H__
#define BLE_DIM_C_H__

#include <stdint.h>
#include "ble.h"

/**
 * @defgroup dim_c_enums Enumerations
 * @{
 */

/**@brief Dimmer Service Client event type. */
typedef enum
{
    BLE_DIM_C_EVT_DISCOVERY_COMPLETE,  /**< Event indicating that the Dimmer Service has been discovered at the peer. */
    BLE_DIM_C_EVT_BATT_NOTIFICATION,   /**< Event indicating that a notification of the Dimmer Level characteristic has been received from the peer. */
    BLE_DIM_C_EVT_BATT_READ_RESP       /**< Event indicating that a read response on Dimmer Level characteristic has been received from peer. */
} ble_dim_c_evt_type_t;

/** @} */

/**
 * @defgroup dim_c_structs Structures
 * @{
 */

/**@brief Dimmer Service Client Event structure. */
typedef struct
{
    ble_dim_c_evt_type_t evt_type;  /**< Event Type. */
    union
    {
        uint8_t dimmer_level;  /**< Dimmer level received from peer. This field will be used for the events @ref BLE_DIM_C_EVT_BATT_NOTIFICATION and @ref BLE_BAS_C_EVT_BATT_READ_RESP.*/
    } params;
} ble_dim_c_evt_t;

/** @} */

/**
 * @defgroup dim_c_types Types
 * @{
 */

// Forward declaration of the ble_dim_t type.
typedef struct ble_dim_c_s ble_dim_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module in order to receive events.
 */
typedef void (* ble_dim_c_evt_handler_t) (ble_dim_c_t * p_dim_dim_c, ble_dim_c_evt_t * p_evt);

/** @} */

/**
 * @addtogroup dim_c_structs
 * @{
 */

/**@brief      Dimmer Service Client structure.

 */
typedef struct ble_dim_c_s
{
    uint16_t                conn_handle;     /**< Connection handle as provided by the SoftDevice. */
    uint16_t                bl_cccd_handle;  /**< Handle of the CCCD of the Dimmer Level characteristic. */
    uint16_t                bl_handle;       /**< Handle of the Dimmer Level characteristic as provided by the SoftDevice. */
    ble_dim_c_evt_handler_t evt_handler;     /**< Application event handler to be called when there is an event related to the Dimmer service. */
} ble_dim_c_t;

/**@brief   Dimmer Service Client initialization structure.
 */
typedef struct
{
    ble_dim_c_evt_handler_t evt_handler;  /**< Event handler to be called by the Dimmer Service Client module whenever there is an event related to the Dimmer Service. */
} ble_dim_c_init_t;

/** @} */

/**
 * @defgroup dim_c_functions Functions
 * @{
 */

/**@brief      Function for initializing the Dimmer Service Client module.
 *
 * @details    This function will initialize the module and set up Database Discovery to discover
 *             the Dimmer Service. After calling this function, call @ref ble_db_discovery_start
 *             to start discovery.
 *
 * @param[out] p_ble_dim_c      Pointer to the Dimmer Service client structure.
 * @param[in]  p_ble_dim_c_init Pointer to the Dimmer Service initialization structure containing
 *                              the initialization information.
 *
 * @retval     NRF_SUCCESS      Operation success.
 * @retval     NRF_ERROR_NULL   A parameter is NULL.
 *                              Otherwise, an error code returned by @ref ble_db_discovery_register.
 */
uint32_t ble_dim_c_init(ble_dim_c_t * p_ble_dim_c, ble_dim_c_init_t * p_ble_dim_c_init);


/**@brief     Function for handling BLE events from the SoftDevice.
 *
 * @details   This function will handle the BLE events received from the SoftDevice. If the BLE
 *            event is relevant for the Dimmer Service Client module, then it is used to update
 *            interval variables and, if necessary, send events to the application.
 *
 * @note      This function must be called by the application.
 *
 * @param[in] p_ble_dim_c Pointer to the Dimmer Service client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event.
 */
void ble_dim_c_on_ble_evt(ble_dim_c_t * p_ble_dim_c, const ble_evt_t * p_ble_evt);


/**@brief   Function for enabling notifications on the Dimmer Level characteristic.
 *
 * @details This function will enable to notification of the Dimmer Level characteristic at the
 *          peer by writing to the CCCD of the Dimmer Level Characteristic.
 *
 * @param   p_ble_dim_c Pointer to the Dimmer Service client structure.
 *
 * @retval  NRF_SUCCESS     If the SoftDevice has been requested to write to the CCCD of the peer.
 *          NRF_ERROR_NULL  Parameter is NULL.
 *                          Otherwise, an error code returned by the SoftDevice API @ref 
 *                          sd_ble_gattc_write.
 */
uint32_t ble_dim_c_bl_notif_enable(ble_dim_c_t * p_ble_dim_c);


/**@brief   Function for reading the Dimmer Level characteristic.
 *
 * @param   p_ble_dim_c Pointer to the Dimmer Service client structure.
 *
 * @retval  NRF_SUCCESS If the read request was successfully queued to be sent to peer.
 */
uint32_t ble_dim_c_bl_read(ble_dim_c_t * p_ble_dim_c);


/** @} */ // End tag for Function group.

#endif // BLE_DIM_C_H__

/** @} */ // End tag for the file.
