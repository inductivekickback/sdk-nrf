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

#define RAD_MSG_TYPE_RAD_MIN_MSG_LEN			20
#define RAD_MSG_TYPE_RAD_MAX_MSG_LEN			20
#define RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_MS		5

#define RAD_MSG_TYPE_DYNASTY_MIN_MSG_LEN		14
#define RAD_MSG_TYPE_DYNASTY_MAX_MSG_LEN		14
#define RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_MS	5

#define RAD_MSG_TYPE_LASER_X_MIN_MSG_LEN		14
#define RAD_MSG_TYPE_LASER_X_MAX_MSG_LEN		14
#define RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_MS	5

typedef enum
{
    PARSE_STATE_INCOMPLETE, /* Not enough of the message has been received. */
    PARSE_STATE_INVALID,    /* The message is not going to work out. */
    PARSE_STATE_VALID,      /* The message is valid. */
    PARSE_STATE_COUNT
} parse_state_t;

// TODO: 

#define RAD_MSG_MAX_LEN        		0
#define RAD_MSG_LINE_CLEAR_LEN_MS 	0

#if CONFIG_RAD_RX_ACCEPT_RAD
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_RAD_MAX_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_RAD_MAX_MSG_LEN
#endif
#if RAD_MSG_LINE_CLEAR_LEN_MS < RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_MS
#undef RAD_MSG_LINE_CLEAR_LEN_MS
#define RAD_MSG_LINE_CLEAR_LEN_MS RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_MS
#endif
#endif

#if CONFIG_RAD_RX_ACCEPT_DYNASTY
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_DYNASTY_MAX_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_DYNASTY_MAX_MSG_LEN
#endif
#if RAD_MSG_LINE_CLEAR_LEN_MS < RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_MS
#undef RAD_MSG_LINE_CLEAR_LEN_MS
#define RAD_MSG_LINE_CLEAR_LEN_MS RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_MS
#endif
#endif

#if CONFIG_RAD_RX_ACCEPT_LASER_X
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_LASER_X_MAX_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_LASER_X_MAX_MSG_LEN
#endif
#if RAD_MSG_LINE_CLEAR_LEN_MS < RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_MS
#undef RAD_MSG_LINE_CLEAR_LEN_MS
#define RAD_MSG_LINE_CLEAR_LEN_MS RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_MS
#endif
#endif

#if RAD_MSG_MAX_LEN == 0
#error No accepted message types enabled.
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_H_ */
 