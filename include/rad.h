/**
 * @file rad.h
 *
 * @brief Public API for the Rad libraries
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_H_
#define ZEPHYR_INCLUDE_RAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>
#include <stdbool.h>

typedef enum
{
    PARSE_STATE_INCOMPLETE, /* Not enough of the message has been received. */
    PARSE_STATE_INVALID,    /* The message is not going to work out. */
    PARSE_STATE_VALID,      /* The message is valid. */
    PARSE_STATE_COUNT
} parse_state_t;

#define RAD_MSG_START_PULSE_MARGIN_US 500 /* A valid start pulse can be +/- this much. */

/**
 * A *_MSG_LEN is the number of elapsed-time measurements required to describe a message.
 *
 * A *_MSG_START_PULSE_LEN_US is the expected length of the start pulse.
 *
 * A *_MSG_LINE_CLEAR_LEN_US is 110% of the expected amount of time that needs to elapse
 * after the final active pulse in order to decide that the message is finished.
 */
#define RAD_MSG_TYPE_RAD_MSG_LEN                    17
#define RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US     1580
#define RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US      860

#define RAD_MSG_TYPE_DYNASTY_MSG_LEN                41
#define RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US 1660
#define RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US  830

#define RAD_MSG_TYPE_LASER_X_MSG_LEN                9
#define RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US 5950
#define RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US  500

#define RAD_MSG_MAX_LEN                0
#define RAD_MSG_LINE_CLEAR_LEN_US      0
#define RAD_MSG_MIN_START_PULSE_LEN_US 100000

#if CONFIG_RAD_RX_ACCEPT_RAD
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_RAD_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_RAD_MSG_LEN
#endif
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US
#endif
#endif

#if CONFIG_RAD_RX_ACCEPT_DYNASTY
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_DYNASTY_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_DYNASTY_MSG_LEN
#endif
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US
#endif
#endif

#if CONFIG_RAD_RX_ACCEPT_LASER_X
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_LASER_X_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_LASER_X_MSG_LEN
#endif
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US
#endif
#endif

#if RAD_MSG_MAX_LEN == 0
#error No accepted message types enabled.
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_H_ */
 