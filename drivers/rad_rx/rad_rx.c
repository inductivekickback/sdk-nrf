/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * TODO: Pin changes are serviced by interrupts so no separate thread is needed.
 *         - This means processing needs to be short.
 *         - Measure the elapsed time and then add item to sys queue
 *         - Then process and create another work queue item to execute callback when necessary.
 *         - But need to reset message index and state from there.
 *       How to synchronize between pin-change interrupt and sys queue processing??????
 *         - If we wait until the line clears then that is added latency.
 *         - If we parse as soon as length is achieved then we need to skip invalid messages.
 *         - Once a message is accepted/rejected by all libs then wait for clear.
 *         - Only try to parse on rising edge.
 */

#define DT_DRV_COMPAT dmv_rad_rx

#include <kernel.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include <rad.h>
#include <drivers/rad_rx.h>

LOG_MODULE_REGISTER(rad_rx, CONFIG_RAD_RX_LOG_LEVEL);

#define IS_VALID_START_PULSE(value, target) ((target)-RAD_MSG_START_PULSE_MARGIN_US >= (value) && \
                                                (target)+RAD_MSG_START_PULSE_MARGIN_US <= (value))

typedef enum
{
    MSG_STATE_WAIT_FOR_LINE_CLEAR,
    MSG_STATE_WAIT_FOR_PREAMBLE,
    MSG_STATE_WAIT_FOR_1,
    MSG_STATE_WAIT_FOR_0,
    MSG_STATE_COMPLETE,
    MSG_STATE_COUNT
} msg_state_t;

struct rad_rx_data {
    bool                  ready;

    const struct device  *dev;
    struct gpio_callback  cb_data;

    rad_rx_callback_t    *cb;

    struct k_timer        timer;
    struct k_work         work;

    uint32_t              message[RAD_MSG_MAX_LEN];
    uint32_t              timestamp;
    atomic_t              index;
    msg_state_t           state;

#if CONFIG_RAD_RX_ACCEPT_RAD
    parse_state_t         rad_parse_state;
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
    parse_state_t         dynasty_parse_state;
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
    parse_state_t         laser_x_parse_state;
#endif
};

struct rad_rx_cfg {
    const char * const port;
    const uint8_t      pin;
    const uint32_t     flags;
};

static void line_clear_timer_expire(struct k_timer *timer_id)
{
    // TODO: Ensure that this interrupt has a higher priority than the GPIO interrupt
    //       to preclude a race condition.
    struct rad_rx_data *data = CONTAINER_OF(timer_id, struct rad_rx_data, timer);

    data->state               = MSG_STATE_WAIT_FOR_PREAMBLE;
    data->index               = 0;
#if CONFIG_RAD_RX_ACCEPT_RAD
    data->rad_parse_state     = PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
    data->dynasty_parse_state = PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
    data->laser_x_parse_state = PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
}

static void message_decode(struct k_work *item)
{
    struct rad_rx_data *data         = CONTAINER_OF(item, struct rad_rx_data, work);
    uint32_t            index        = atomic_get(&data->index);
    bool                msg_finished = true;

    if (MSG_STATE_WAIT_FOR_LINE_CLEAR == data->state) {
        /* There might be one additional input change after giving up on the message. */
        return;
    }

#if CONFIG_RAD_RX_ACCEPT_RAD
    switch (data->rad_parse_state) {
    case PARSE_STATE_WAIT_FOR_START_PULSE:
        if (!IS_VALID_START_PULSE(data->message[0], RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US)) {
            data->rad_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        } else {
            msg_finished          = false;
            data->rad_parse_state = RAD_PARSE_STATE_INCOMPLETE;
            // Fall-through into the next state in case the entire message is received.
        }
    case PARSE_STATE_INCOMPLETE:
        if (RAD_MSG_TYPE_RAD_MSG_LEN > index) {
            msg_finished = false;
            break;
        } else if (RAD_MSG_TYPE_RAD_MSG_LEN < index) {
            data->rad_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        }

        if (RAD_PARSE_STATE_VALID == rad_message_type_rad_parse()) {
            // TODO: Execute callback: data->cb(RAD_MSG_TYPE_RAD, pointer to data);
            data->rad_parse_state = RAD_PARSE_STATE_VALID;
        }
        break;
    default:
        break;
    }
#endif

    if (msg_finished) {
        data->state = MSG_STATE_WAIT_FOR_LINE_CLEAR;
        k_timer_start(&data->timer, K_MSEC(RAD_MSG_LINE_CLEAR_LEN_US), K_NO_WAIT);
    }
}

static void input_changed(const struct device *dev, struct gpio_callback *cb_data, uint32_t pins)
{
    struct rad_rx_data *data = CONTAINER_OF(cb_data, struct rad_rx_data, cb_data);

    if (MSG_STATE_WAIT_FOR_LINE_CLEAR == data->state) {
        k_timer_start(&data->timer, K_MSEC(RAD_MSG_LINE_CLEAR_LEN_US), K_NO_WAIT);
        return;
    }

    uint32_t now   = k_cycle_get_32();
    uint32_t index = atomic_inc(&data->index); /* index is set to the pre-incremented valued */

    if (0 < index) {
        if (RAD_MSG_MAX_LEN <= index) {
            return;
        }
        if (data->timestamp <= now) {
            data->message[index-1] = (now - data->timestamp);
        } else {
            data->message[index-1] = (0xFFFFFFFF - data->timestamp);
            data->message[index-1] += now;
        }

        // Only attempt to decode on rising edges.
        if (gpio_pin_get(dev, pins)) {
            k_work_submit(&data->work);
        }
    }
    data->timestamp = now;
}

static int dmv_rad_rx_init(const struct device *dev)
{
    int err;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static int dmv_rad_set_callback(rad_rx_callback_t *cb)
{
    struct rad_rx_data *data = CONTAINER_OF(cb_data, struct rad_rx_data, cb_data);

    data->cb = cb;
    return 0;
}

static const struct rad_rx_driver_api rad_rx_driver_api = {
    .init         = dmv_rad_rx_init,
    .set_callback = dmv_rad_set_callback,
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
