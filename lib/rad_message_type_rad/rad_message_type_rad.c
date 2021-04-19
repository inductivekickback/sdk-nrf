/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * 38KHz carrier
 * Want a minimum burst length of 10 cycles
 * Use gap of at least 10 cycles for short ons (10 to 35)
 * Use gap of 10x for longer ons 
 * Max number of on bursts is 1500 per second
 */

#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <rad.h>

LOG_MODULE_REGISTER(rad_message_type_rad, CONFIG_RAD_MESSAGE_TYPE_RAD_LOG_LEVEL);

parse_state_t rad_message_type_rad_parse(uint32_t *message, uint32_t len)
{
	// TODO: Compare the preamble so it fails faster?

	if (RAD_MSG_TYPE_RAD_MIN_MSG_LEN > len) {
		return PARSE_STATE_INCOMPLETE;
	}

	if (RAD_MSG_TYPE_RAD_MAX_MSG_LEN < len) {
		return PARSE_STATE_INVALID;
	}

	// TODO: Actually verify it.

	return PARSE_STATE_VALID;
}
