/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * TODO: Pin changes are serviced by interrupts so no separate thread is needed.
 *         - This means processing needs to be short.
 *         - Measure the elapsed time and then add item to sys queue
 *         - Then process and create another work queue item to execute callback.
 *         - But need to reset message index and state from there.
 *       How to synchronize between pin-change interrupt and sys queue processing??????
 */

#define DT_DRV_COMPAT dmv_rad_rx

#include <kernel.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include <drivers/rad_rx.h>

// TODO: These needs to be defined according to which message types are enabled.
#define MAX_MSG_TIMESTAMPS 40
#define LINE_CLEAR_MS      5

LOG_MODULE_REGISTER(rad_rx, CONFIG_RAD_RX_LOG_LEVEL);

typedef enum
{
    MSG_STATE_WAIT_FOR_LINE_CLEAR,
    MSG_STATE_WAIT_FOR_PREAMBLE,
    MSG_STATE_PREAMBLE_RECEIVED,
    MSG_STATE_WAIT_FOR_1,
    MSG_STATE_WAIT_FOR_0,
    MSG_STATE_COMPLETE,
    MSG_STATE_COUNT
} msg_state_t;

struct rad_rx_data {
    bool                  ready;

    const struct device  *dev;
    struct gpio_callback  cb_data;

    struct k_timer        timer;
    struct k_work         work;

    uint32_t              message[MAX_MSG_TIMESTAMPS];
    atomic_t              index;
    msg_state_t           state;
};

struct rad_rx_cfg {
    const char * const port;
    const uint8_t      pin;
    const uint32_t     flags;
};

static void message_decode(struct k_work *item)
{
    // TODO: Interrupt auto increments index so it could be above MAX.
    // TODO: If the message buffer is full then this work won't be queued again.
    // TODO: This might be one more pin change after WAIT_FOR_LINE_CLEAR
    struct rad_rx_data *data = CONTAINER_OF(item, struct rad_rx_data, work);

    // Needs to be re-entrant.
    // Needs to assume that index can change at any time.
    uint32_t index = atomic_get(&data->index);

    // What's the worst case here?
    //   If the message has an error but isn't MAX length then the interrupt adds an entry
    //       But it's waiting for line clear now or soon anyway
    //       If the index hasn't been reset then the new initial timestamp is discarded
}

static void input_changed(const struct device *dev, struct gpio_callback *cb_data, uint32_t pins)
{
    struct rad_rx_data *data = CONTAINER_OF(cb_data, struct rad_rx_data, cb_data);

    if (MSG_STATE_WAIT_FOR_LINE_CLEAR == data->state) {
        k_timer_start(&data->timer, K_MSEC(LINE_CLEAR_MS), K_NO_WAIT);
        return;
    }

    uint32_t index = atomic_inc(&data->index);
    if (MAX_MSG_TIMESTAMPS > index) {
        data->message[index] = k_cycle_get_32();
        k_work_submit(&data->work);
    }
}

static int dmv_rad_rx_init(const struct device *dev)
{
    int err;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static const struct rad_rx_driver_api rad_rx_driver_api = {
    .init  = dmv_rad_rx_init,
};

#define INST(num) DT_INST(num, dmv_rad_rx)

#define RAD_RX_DEVICE(n) \
    static const struct rad_rx_cfg rad_rx_cfg_##n = { \
        .port  = DT_GPIO_LABEL(INST(n), gpios), \
        .pin   = DT_GPIO_PIN(INST(n),   gpios), \
    }; \
    static struct rad_rx_data rad_rx_data_##n; \
    DEVICE_DEFINE(rad_rx_##n, \
                DT_LABEL(INST(n)), \
                dmv_rad_rx_init, \
                NULL, \
                &rad_rx_data_##n, \
                &rad_rx_cfg_##n, \
                POST_KERNEL, \
                CONFIG_RAD_RX_INIT_PRIORITY, \
                &rad_rx_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAD_RX_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Rad laser tag receiver driver enabled without any devices"
#endif
