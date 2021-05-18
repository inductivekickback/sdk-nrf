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

typedef enum
{
	RAD_MSG_TYPE_RAD,
	RAD_MSG_TYPE_DYNASTY,
	RAD_MSG_TYPE_LASER_X,
	RAD_MSG_TYPE_COUNT
} rad_msg_type_t;

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

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_H_ */
 