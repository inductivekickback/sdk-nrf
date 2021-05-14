/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <rad.h>

LOG_MODULE_REGISTER(rad_message_type_rad, CONFIG_RAD_MSG_TYPE_RAD_LOG_LEVEL);


rad_parse_state_t rad_message_type_rad_parse(uint32_t      *message,
	                                         uint32_t       len,
	                                         rad_msg_rad_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     */

    if (RAD_MSG_TYPE_RAD_MSG_LEN > len) {
        return RAD_PARSE_STATE_INCOMPLETE;
    }

    // TODO: Actually verify it.

    return RAD_PARSE_STATE_INVALID;
}
