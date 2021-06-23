/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <drivers/uart.h>

#include <device.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <mpsl.h>
#include <mpsl_radio_notification.h>
#include <mpsl_timeslot.h>

#include <settings/settings.h>

#include <stdio.h>

#include <logging/log.h>

#include <hal/nrf_gpio.h>

#define RADIO_NOTIFICATION_PIN 2

#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static mpsl_timeslot_session_id_t mpsl_session_id;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Connected %s", log_strdup(addr));

	current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Disconnected: %s (reason %u)", log_strdup(addr), reason);

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected    = connected,
	.disconnected = disconnected,
};

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
			  uint16_t len)
{
	char addr[BT_ADDR_LE_STR_LEN] = {0};

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

	LOG_INF("Received data from: %s", log_strdup(addr));
}

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

void error(void)
{
	while (true) {
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
}

static mpsl_timeslot_signal_return_param_t no_action = {
	.callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_NONE
};

 static mpsl_timeslot_signal_return_param_t*
 mpsl_cb(mpsl_timeslot_session_id_t session_id, uint32_t signal)
 {
 	switch (signal) {
 	case MPSL_TIMESLOT_SIGNAL_START:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_TIMER0:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_RADIO:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_EXTEND_FAILED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_EXTEND_SUCCEEDED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_BLOCKED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_CANCELLED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_SESSION_IDLE:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_INVALID_RETURN:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_SESSION_CLOSED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_OVERSTAYED:
 		break;
 	default:
 		break;
 	};

 	return &no_action;
 }

static void radio_notify_cb(const void *context)
{
	nrf_gpio_pin_toggle(RADIO_NOTIFICATION_PIN);
}

void main(void)
{
	int err = 0;

	nrf_gpio_cfg_output(RADIO_NOTIFICATION_PIN);
	nrf_gpio_pin_clear(RADIO_NOTIFICATION_PIN);

	bt_conn_cb_register(&conn_callbacks);

	if (!mpsl_is_initialized()) {
		LOG_ERR("MPSL is not initialized");
		error();
	}

	uint8_t mpsl_rev;
	err = mpsl_build_revision_get(&mpsl_rev);
	if (err) {
		LOG_ERR("mpsl_build_revision_get failed (err: %d)", err);
    	error();
	} else {
		LOG_INF("MPSL build rev: %d", mpsl_rev);
	}

	err = mpsl_radio_notification_cfg_set(MPSL_RADIO_NOTIFICATION_TYPE_INT_ON_BOTH,
			 MPSL_RADIO_NOTIFICATION_DISTANCE_200US,
			 QDEC_IRQn);
	if (err) {
		LOG_ERR("mpsl_radio_notification_cfg_set failed (err: %d)", err);
    	error();
	}

	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(qdec)), 5, radio_notify_cb, NULL, 0);
	irq_enable(DT_IRQN(DT_NODELABEL(qdec)));

	err = mpsl_timeslot_session_open(mpsl_cb, &mpsl_session_id);
	if (err) {
		LOG_ERR("mpsl_timeslot_session_open failed (err: %d)", err);
		error();
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("bt_enable failed (err: %d)", err);
		error();
	}

	LOG_INF("Bluetooth initialized");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_nus_init(&nus_cb);
	if (err) {
		LOG_ERR("Failed to initialize UART service (err: %d)", err);
		error();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
			      ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		error();
	}

	for (;;) {
		k_sleep(K_MSEC(1000));
	}
}
