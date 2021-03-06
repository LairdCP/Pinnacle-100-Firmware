/**
 * @file gateway_fsm.h
 * @brief Network and cloud abstraction layer for gateway.
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __GATEWAY_FSM_H__
#define __GATEWAY_FSM_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Initialize the gateway state machine.
 */
void gateway_fsm_init(void);

/**
 * @brief The gateway state machine must be periodically run.
 * For example, once per second.
 */
void gateway_fsm(void);

/**
 * @brief Request decommissioning.  This will cause the cloud connection
 * to close and the certificates to be unloaded.
 */
void gateway_fsm_request_decommission(void);

/**
 * @brief Request the closure of the cloud connection.
 * It will be reopened after a delay.
 */
void gateway_fsm_request_cloud_disconnect(void);

/**
 * @brief Weak implementations that can be overriden by application
 */
bool gateway_fsm_fota_request(void);

/**
 * @brief Weak implementations that can be used by application
 *
 * @note if error callback returns 0 then network will be reinitialized.
 * However, for the HL7800 a reset is recommended.
 */
int gateway_fsm_network_error_callback(void);
void gateway_fsm_modem_init_complete_callback(void);
void gateway_fsm_network_init_complete_callback(void);
void gateway_fsm_network_connected_callback(void);
void gateway_fsm_network_disconnected_callback(void);
void gateway_fsm_cloud_connected_callback(void);
void gateway_fsm_cloud_disconnected_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __GATEWAY_FSM_H__ */
