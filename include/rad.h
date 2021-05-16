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
    RAD_PARSE_STATE_WAIT_FOR_START_PULSE, /* A valid start pulse is required before parsing. */
    RAD_PARSE_STATE_INCOMPLETE,           /* Not enough of the message has been received. */
    RAD_PARSE_STATE_INVALID,              /* The message is not going to work out. */
    RAD_PARSE_STATE_VALID,                /* The message is valid. */
    RAD_PARSE_STATE_COUNT
} rad_parse_state_t;

typedef enum
{
	RAD_MSG_TYPE_RAD,
	RAD_MSG_TYPE_DYNASTY,
	RAD_MSG_TYPE_LASER_X,
	RAD_MSG_TYPE_COUNT
} rad_msg_type_t;

#define RAD_MSG_START_PULSE_MARGIN_US 500 /* A valid start pulse can be +/- this much. */
#define RAD_MSG_BIT_MARGIN_US         125 /* A valid bit pulse can be +/- this much. */

typedef struct
{
	uint8_t damage		: 4;
	uint8_t special 	: 4;
	uint8_t player_id	: 4;
	uint8_t team_id		: 2;
	uint8_t reserved	: 2;
} rad_msg_rad_t;

typedef enum
{
	TEAM_ID_LASER_X_BLUE    = 0x51,
	TEAM_ID_LASER_X_RED     = 0x52,
	TEAM_ID_LASER_X_NEUTRAL = 0x53
} team_id_laser_x_t;

typedef struct
{
	uint8_t team_id;
} rad_msg_laser_x_t;

typedef enum
{
	TEAM_ID_DYNASTY_BLUE  = 1,
	TEAM_ID_DYNASTY_RED   = 2,
	TEAM_ID_DYNASTY_GREEN = 3,
    TEAM_ID_DYNASTY_WHITE = 4
} team_id_dynasty_t;

typedef enum
{
	WEAPON_ID_DYNASTY_PISTOL      = 1,
	WEAPON_ID_DYNASTY_SHOTGUN_SMG = 2,
	WEAPON_ID_DYNASTY_ROCKET      = 3
} weapon_id_dynasty_t;

typedef struct
{
	uint8_t team_id;
	uint8_t weapon_id;
	uint8_t checksum;
} rad_msg_dynasty_t;

/**
 * A *_MSG_LEN is the number of elapsed-time measurements required to describe a message, including
 * the start pulse.
 *
 * A *_MSG_START_PULSE_LEN_US is the expected length of the start pulse.
 *
 * A *_MSG_LINE_CLEAR_LEN_US is the expected amount of time that needs to elapse after the final
 * active pulse in order to decide that the message is finished.
 */
#define RAD_MSG_TYPE_RAD_MSG_LEN                    37
#define RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US     1580
#define RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US      860
#define RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US           390
#define RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US           780
#define RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US      390

#define RAD_MSG_TYPE_DYNASTY_MSG_LEN                41
#define RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US 1660
#define RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US  900
#define RAD_MSG_TYPE_DYNASTY_MSG_0_BIT_LEN_US    	412
#define RAD_MSG_TYPE_DYNASTY_MSG_1_BIT_LEN_US       747

#define RAD_MSG_TYPE_LASER_X_MSG_LEN                17
#define RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US 5950
#define RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US  600
#define RAD_MSG_TYPE_LASER_X_MSG_SPACE_BIT_LEN_US   450
#define RAD_MSG_TYPE_LASER_X_MSG_0_BIT_LEN_US    	550
#define RAD_MSG_TYPE_LASER_X_MSG_1_BIT_LEN_US       1525

#define RAD_MSG_MAX_LEN                             0
#define RAD_MSG_LINE_CLEAR_LEN_US                   0
#define RAD_MSG_MIN_START_PULSE_LEN_US              100000


#if CONFIG_RAD_MSG_TYPE_RAD
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_RAD_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_RAD_MSG_LEN
#endif
#if CONFIG_RAD_RX_ACCEPT_RAD
rad_parse_state_t rad_msg_type_rad_parse(uint32_t      *message,
	                                     uint32_t       len,
	                                     rad_msg_rad_t *msg);
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_RAD_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_RAD_MSG_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_RAD */
#endif /* CONFIG_RAD_MSG_TYPE_RAD */


#if CONFIG_RAD_MSG_TYPE_DYNASTY
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_DYNASTY_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_DYNASTY_MSG_LEN
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
rad_parse_state_t rad_msg_type_dynasty_parse(uint32_t          *message,
        	                                 uint32_t           len,
	                                         rad_msg_dynasty_t *msg);
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_DYNASTY_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_DYNASTY_MSG_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_DYNASTY */
#endif /* CONFIG_RAD_MSG_TYPE_DYNASTY */


#if CONFIG_RAD_MSG_TYPE_LASER_X
#if RAD_MSG_MAX_LEN < RAD_MSG_TYPE_LASER_X_MSG_LEN
#undef RAD_MSG_MAX_LEN
#define RAD_MSG_MAX_LEN RAD_MSG_TYPE_LASER_X_MSG_LEN
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
rad_parse_state_t rad_msg_type_laser_x_parse(uint32_t          *message,
        	                                 uint32_t           len,
	                                         rad_msg_laser_x_t *msg);
#if RAD_MSG_MIN_START_PULSE_LEN_US > RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US
#undef RAD_MSG_MIN_START_PULSE_LEN_US
#define RAD_MSG_MIN_START_PULSE_LEN_US RAD_MSG_TYPE_LASER_X_MSG_START_PULSE_LEN_US
#endif
#if RAD_MSG_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US
#undef RAD_MSG_LINE_CLEAR_LEN_US
#define RAD_MSG_LINE_CLEAR_LEN_US RAD_MSG_TYPE_LASER_X_MSG_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_LASER_X */
#endif /* CONFIG_RAD_MSG_TYPE_LASER_X */

#if CONFIG_RAD_TX
#if RAD_MSG_MAX_LEN == 0
#error No Rad TX message types enabled
#endif
#endif

#if CONFIG_RAD_RX
#if RAD_MSG_LINE_CLEAR_LEN_US == 0
#error No Rad RX message types enabled
#endif
#endif

#define IS_VALID_START_PULSE(value, target) ((target)-RAD_MSG_START_PULSE_MARGIN_US <= (value) && \
                                                (target)+RAD_MSG_START_PULSE_MARGIN_US >= (value))

#define IS_VALID_BIT_PULSE(value, target) ((target)-RAD_MSG_BIT_MARGIN_US <= (value) && \
                                                (target)+RAD_MSG_BIT_MARGIN_US >= (value))

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_H_ */
 