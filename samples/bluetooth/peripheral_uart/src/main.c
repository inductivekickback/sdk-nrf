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

#include <settings/settings.h>

#include <stdio.h>

#include <logging/log.h>

#include <hal/nrf_gpio.h>

#include <timeslot.h>

#define TS_LEN_US           1500
#define TS_REQUEST_DELAY_US 2100
#define RNH_SETTLE_COUNT    2

#define CI_TO_US(ci_ms)     (1250UL * (ci_ms))

#define RADIO_NOTIFICATION_PIN 2
#define REQUEST_PIN            31

#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static uint16_t ts_conn_interval;
static uint16_t ts_next_interval;
static uint32_t ts_rnh_delay;
static bool     ts_ready_to_start;

static struct k_poll_signal timeslot_sig = K_POLL_SIGNAL_INITIALIZER(timeslot_sig);
static struct k_poll_event  events[1]    = {
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SIGNAL,
                                    K_POLL_MODE_NOTIFY_ONLY,
                                    &timeslot_sig, 0),
};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

void error(void)
{
    LOG_ERR("Error handler");
    while (true) {
        /* Spin for ever */
        k_sleep(K_MSEC(1000));
    }
}

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

    ts_conn_interval = ts_next_interval = 0;
    int err = timeslot_stop();
    if (err) {
        LOG_ERR("timeslot_stop failed (err=%d)", err);
        error();
    }
}

static void conn_param_updated(struct bt_conn *conn, uint16_t interval,
                 uint16_t latency, uint16_t timeout)
{
    /* NOTE: This may be called multiple times at the beginning of the connection. */
    LOG_INF("Connection params updated: (interval=%d, SL=%d, timeout=%d)",
                interval, latency, timeout);

    if (ts_conn_interval) {
        /* This isn't the first conn_param_update. */
        if (interval != ts_conn_interval) {
            LOG_INF("Stopping current timeslot");
            ts_next_interval = interval;
            int err          = timeslot_stop();
            if (err) {
                LOG_ERR("timeslot_stop failed (err=%d)", err);
                error();
            }
        }
    } else {
        LOG_INF("Starting timeslot");
        ts_next_interval  = interval;
        ts_rnh_delay      = RNH_SETTLE_COUNT;
        ts_ready_to_start = true;
    }
}

static struct bt_conn_cb conn_callbacks = {
    .connected        = connected,
    .disconnected     = disconnected,
    .le_param_updated = conn_param_updated,
};

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
              uint16_t len)
{
    char addr[BT_ADDR_LE_STR_LEN] = {0};

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

    LOG_INF("Received data from: %s", log_strdup(addr));
}

static void bt_nus_enabled_cb(enum bt_nus_send_status status)
{
    switch (status) {
    case BT_NUS_SEND_STATUS_ENABLED:
        LOG_INF("NUS TX CCCD enabled");
        break;
    case BT_NUS_SEND_STATUS_DISABLED:
        LOG_INF("NUX TX CCCD disabled");
        break;
    default:
        break;
    }
}

static struct bt_nus_cb nus_cb = {
    .received     = bt_receive_cb,
    .send_enabled = bt_nus_enabled_cb,
};

static void radio_notify_cb(const void *context)
{
    static bool active;

    active = !active;
    nrf_gpio_pin_write(RADIO_NOTIFICATION_PIN, active);

    if (ts_ready_to_start && !active) {
        if (ts_rnh_delay) {
            ts_rnh_delay--;
        } else {
            ts_ready_to_start = false;
            k_poll_signal_raise(&timeslot_sig, 0);
        }
    }
}

static void timeslot_err_cb(int err)
{
    LOG_ERR("Timeslot session error: %d", err);
    error();
}

static void timeslot_start_cb(void)
{
    LOG_DBG("Timeslot start");
}

static void timeslot_end_cb(void)
{
    LOG_DBG("Timeslot end");
}

static void timeslot_skipped_cb(uint8_t count)
{
    LOG_INF("Timeslot skipped: %d", count);
}

static void timeslot_stopped_cb(void)
{
    LOG_INF("Timeslot stopped");
    if (ts_conn_interval != ts_next_interval) {
        LOG_INF("Restarting timeslot");
        ts_rnh_delay     = RNH_SETTLE_COUNT;
        ts_ready_to_start = true;
    }
}

#if !TIMESLOT_CALLS_RADIO_IRQHANDLER
static void radio_irq_cb(void)
{
    LOG_DBG("Radio_IRQHandler");
}
#endif

static struct timeslot_cb timeslot_callbacks = {
    .error     = timeslot_err_cb,
    .start     = timeslot_start_cb,
    .end       = timeslot_end_cb,
    .skipped   = timeslot_skipped_cb,
    .stopped   = timeslot_stopped_cb,
#if !TIMESLOT_CALLS_RADIO_IRQHANDLER
    .radio_irq = radio_irq_cb
#endif
};

static struct timeslot_config timeslot_config = TS_DEFAULT_CONFIG;

void main(void)
{
    int err = 0;

    nrf_gpio_cfg_output(RADIO_NOTIFICATION_PIN);
    nrf_gpio_cfg_output(REQUEST_PIN);
    nrf_gpio_pin_clear(RADIO_NOTIFICATION_PIN);
    nrf_gpio_pin_clear(REQUEST_PIN);

    bt_conn_cb_register(&conn_callbacks);

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

    err = timeslot_open(&timeslot_config, &timeslot_callbacks);
    if (err) {
        LOG_ERR("timeslot_open failed (err: %d)", err);
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

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        error();
    }

    for (;;) {
        k_poll(events, 1, K_FOREVER);

        nrf_gpio_pin_write(REQUEST_PIN, 1);
        k_sleep(K_USEC(CONFIG_SDC_MAX_CONN_EVENT_LEN_DEFAULT - TS_REQUEST_DELAY_US));
        nrf_gpio_pin_write(REQUEST_PIN, 0);

        ts_conn_interval = ts_next_interval;
        int err = timeslot_start(TS_LEN_US, CI_TO_US(ts_conn_interval));
        if (err) {
            LOG_ERR("timeslot_start failed (err=%d)", err);
            error();
        }

        events[0].signal->signaled = 0;
        events[0].state            = K_POLL_STATE_NOT_READY;
    }
}
