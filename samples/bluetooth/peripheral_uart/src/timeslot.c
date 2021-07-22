/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>

#include <logging/log.h>

#define LOG_MODULE_NAME timeslot
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <mpsl.h>
#include <mpsl_timeslot.h>

#define TS_GPIO_DEBUG 1

#if TS_GPIO_DEBUG
#include <hal/nrf_gpio.h>
#define TIMESLOT_PIN 4
#endif

#include <timeslot.h>

#define PROPRIETARY_RF_THREAD_STACK_SIZE 768
#define PROPRIETARY_RF_THREAD_PRIORITY   5

enum SIGNAL_CODE
{
    SIGNAL_CODE_START             = 0x00,
    SIGNAL_CODE_TIMER0            = 0x01,
    SIGNAL_CODE_RADIO             = 0x02,
    SIGNAL_CODE_BLOCKED_CANCELLED = 0x03,
    SIGNAL_CODE_OVERSTAYED        = 0x04,
    SIGNAL_CODE_IDLE              = 0x05,
    SIGNAL_CODE_UNEXPECTED        = 0x06,
    SIGNAL_CODE_START_FROM_ISR    = 0x07
};

static uint32_t                conn_interval_us;
static uint32_t                ts_len_us;
static uint8_t                 blocked_cancelled_count;
static bool                    session_open;
static bool                    timeslot_started;
static bool                    timeslot_stopping;
static struct timeslot_config *timeslot_config;
static struct timeslot_cb     *timeslot_callbacks;

static struct k_poll_signal timeslot_sig = K_POLL_SIGNAL_INITIALIZER(timeslot_sig);
static struct k_poll_event  events[1] = {
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SIGNAL,
                                    K_POLL_MODE_NOTIFY_ONLY,
                                    &timeslot_sig, 0),
};

static mpsl_timeslot_session_id_t mpsl_session_id;

/* NOTE: MPSL return params must be in static scope. */
static mpsl_timeslot_request_t request_earliest = {
    .request_type = MPSL_TIMESLOT_REQ_TYPE_EARLIEST,
    .params.earliest = {
        .priority   = MPSL_TIMESLOT_PRIORITY_NORMAL,
    }
};

static mpsl_timeslot_request_t request_normal = {
    .request_type = MPSL_TIMESLOT_REQ_TYPE_NORMAL
};

static mpsl_timeslot_signal_return_param_t action_none = {
    .callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_NONE
};

static mpsl_timeslot_signal_return_param_t action_end = {
    .callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_END
};

static mpsl_timeslot_signal_return_param_t action_request_normal = {
    .callback_action       = MPSL_TIMESLOT_SIGNAL_ACTION_REQUEST,
    .params.request.p_next = &request_normal
};

static mpsl_timeslot_signal_return_param_t*
mpsl_cb(mpsl_timeslot_session_id_t session_id, uint32_t signal)
{
    /**
     *  NOTE: The MPSL_TIMESLOT_SIGNAL_START, MPSL_TIMESLOT_SIGNAL_TIMER0, and
     *        MPSL_TIMESLOT_SIGNAL_RADIO signals are called from an ISR at priority zero.
     */
    switch (signal) {
    case MPSL_TIMESLOT_SIGNAL_START:
        blocked_cancelled_count=0;
#if TS_GPIO_DEBUG
        nrf_gpio_pin_write(TIMESLOT_PIN, 1);
#endif
        if (timeslot_stopping) {
            return &action_end;
        }
        /* TIMER0 is pre-configured for 1MHz mode by the MPSL. */
        NRF_TIMER0->INTENSET = (TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos);
        NRF_TIMER0->CC[0]    = (ts_len_us - timeslot_config->safety_margin_us);
        NVIC_EnableIRQ(TIMER0_IRQn);
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_START);
        break;

    case MPSL_TIMESLOT_SIGNAL_TIMER0:
#if TS_GPIO_DEBUG
        nrf_gpio_pin_write(TIMESLOT_PIN, 0);
#endif
        if (timeslot_stopping) {
            return &action_end;
        }
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_TIMER0);
        request_normal.params.normal.distance_us = conn_interval_us;
        request_normal.params.normal.priority    = MPSL_TIMESLOT_PRIORITY_NORMAL;
        return &action_request_normal;

    case MPSL_TIMESLOT_SIGNAL_RADIO:
        if (timeslot_stopping) {
            return &action_end;
        }
#if TIMESLOT_CALLS_RADIO_IRQHANDLER
        RADIO_IRQHandler();
#else
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_RADIO);
#endif
        break;

    case MPSL_TIMESLOT_SIGNAL_BLOCKED:
        blocked_cancelled_count++;
        if (timeslot_stopping) {
            return &action_end;
        }
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_BLOCKED_CANCELLED);
        break;

    case MPSL_TIMESLOT_SIGNAL_CANCELLED:
        blocked_cancelled_count++;
        if (timeslot_stopping) {
            return &action_end;
        }
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_BLOCKED_CANCELLED);
        break;

    case MPSL_TIMESLOT_SIGNAL_SESSION_IDLE:
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_IDLE);
        break;

    case MPSL_TIMESLOT_SIGNAL_EXTEND_FAILED:
        /* Intentional fall-through */
    case MPSL_TIMESLOT_SIGNAL_EXTEND_SUCCEEDED:
        /* Intentional fall-through */
    case MPSL_TIMESLOT_SIGNAL_INVALID_RETURN:
        /* Intentional fall-through */
    case MPSL_TIMESLOT_SIGNAL_SESSION_CLOSED:
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_UNEXPECTED);
        break;

    case MPSL_TIMESLOT_SIGNAL_OVERSTAYED:
        k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_OVERSTAYED);
        break;

    default:
        break;
    };

    return &action_none;
}

int timeslot_stop(void)
{
    if (!session_open || !timeslot_started) {
        return -1;
    }
    timeslot_stopping = true;
    return 0;
}

int timeslot_start(uint32_t len_us, uint32_t interval_us)
{
    if (!session_open || timeslot_started || timeslot_stopping) {
        return -1;
    }

    LOG_INF("timeslot_start (len_us: %d, interval_us: %d)", len_us, interval_us);
    ts_len_us               = len_us;
    conn_interval_us        = interval_us;
    blocked_cancelled_count = 0;
    timeslot_started        = true;

    request_normal.params.normal.length_us     = len_us;
    request_earliest.params.earliest.length_us = len_us;

    if (!k_is_in_isr()) {
        LOG_INF("Not in ISR");
        return mpsl_timeslot_request(mpsl_session_id, &request_earliest);
    }

    k_poll_signal_raise(&timeslot_sig, SIGNAL_CODE_START_FROM_ISR);
    return 0;
}

int timeslot_open(struct timeslot_config *config, struct timeslot_cb *cb)
{
    if (session_open) {
        return -1;
    }

    if (0 == config) {
        return -2;
    }

    if ((0 == cb) || (0 == cb->error) || (0 == cb->start) || (0 == cb->end)) {
        return -2;
    }
#if !TIMESLOT_CALLS_RADIO_IRQHANDLER
    if (0 == cb->radio_irq) {
        return -2;
    }
#endif

    timeslot_config    = config;
    timeslot_callbacks = cb;

    request_normal.params.normal.hfclk          = timeslot_config->hfclk;
    request_earliest.params.earliest.hfclk      = timeslot_config->hfclk;
    request_earliest.params.earliest.timeout_us = timeslot_config->timeout_us;

    int err = mpsl_timeslot_session_open(mpsl_cb, &mpsl_session_id);
    if (err) {
        return err;
    }

#if TS_GPIO_DEBUG
    nrf_gpio_cfg_output(TIMESLOT_PIN);
    nrf_gpio_pin_clear(TIMESLOT_PIN);
#endif

    session_open = true;
    return 0;
}

static void timeslot_thread_fn(void)
{
    int err;

    while (true) {
        k_poll(events, 1, K_FOREVER);

        switch (events[0].signal->result) {
        case SIGNAL_CODE_START:
            timeslot_callbacks->start();
            break;

        case SIGNAL_CODE_TIMER0:
            timeslot_callbacks->end();
            break;

#if !TIMESLOT_CALLS_RADIO_IRQHANDLER
        case SIGNAL_CODE_RADIO:
            timeslot_callbacks->radio_irq();
            break;
#endif

        case SIGNAL_CODE_BLOCKED_CANCELLED:
            LOG_DBG("SIGNAL_CODE_BLOCKED_CANCELLED");
            if (blocked_cancelled_count > timeslot_config->skipped_tolerance) {
                timeslot_callbacks->error(-95);
                return;
            }
            request_normal.params.normal.distance_us = (conn_interval_us * (blocked_cancelled_count+1));
            request_normal.params.normal.priority    = MPSL_TIMESLOT_PRIORITY_HIGH;
            err = mpsl_timeslot_request(mpsl_session_id, &request_normal);
            if (err) {
                timeslot_started  = false;
                timeslot_stopping = false;
                timeslot_callbacks->error(err);
            }
            timeslot_callbacks->skipped(blocked_cancelled_count);
            break;

        case SIGNAL_CODE_IDLE:
            LOG_INF("SIGNAL_CODE_IDLE");
            if (timeslot_stopping) {
                timeslot_stopping = false;
                timeslot_started  = false;
                timeslot_callbacks->stopped();
            } else {
                /* Session ended unexpectedly */
                timeslot_callbacks->error(-96);
            }
            break;

        case SIGNAL_CODE_OVERSTAYED:
            /* This is the most probable of the what-could-go-wrong scenarios. */
            timeslot_callbacks->error(-97);
            break;

        case SIGNAL_CODE_UNEXPECTED:
            /* Something like MPSL_TIMESLOT_SIGNAL_INVALID_RETURN happened. */
            timeslot_callbacks->error(-98);
            break;

        case SIGNAL_CODE_START_FROM_ISR:
            err = mpsl_timeslot_request(mpsl_session_id, &request_earliest);
            if (err) {
                timeslot_started  = false;
                timeslot_stopping = false;
                timeslot_callbacks->error(err);
            }
            break;

        default:
            timeslot_callbacks->error(-99);
            break;
        }

        events[0].signal->signaled = 0;
        events[0].state            = K_POLL_STATE_NOT_READY;
    }
}

K_THREAD_DEFINE(timeslot_thread, PROPRIETARY_RF_THREAD_STACK_SIZE,
                    timeslot_thread_fn, NULL, NULL, NULL,
                    K_PRIO_COOP(PROPRIETARY_RF_THREAD_PRIORITY), 0, 0);
