/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <rad.h>

LOG_MODULE_REGISTER(rad_message_type_laser_x, CONFIG_RAD_MSG_TYPE_LASER_X_LOG_LEVEL);

rad_parse_state_t rad_message_type_laser_x_parse(uint32_t          *message,
                                                 uint32_t           len,
                                                 rad_msg_laser_x_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by eight bits:
     *     START: Active pulse of ~5.95ms
     *     0:     Inactive for ~0.45ms followed by an active pulse of ~0.55ms
     *     1:     Inactive for ~0.45ms followed by an active pulse of ~1.5ms
     */
    msg->team_id = 0;
    for (int i=1,j=7; j>=0; i+=2,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_LASER_X_MSG_SPACE_BIT_LEN_US)) {
            if (IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_LASER_X_MSG_0_BIT_LEN_US)) {
                // This is a zero bit.
                continue;
            } else if (IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_LASER_X_MSG_1_BIT_LEN_US)) {
                // This is a one bit.
                msg->team_id |= (1<<j);
                continue;
            }
        }
        return RAD_PARSE_STATE_INVALID;
    }

    switch (msg->team_id) {
    case TEAM_ID_LASER_X_BLUE:
    case TEAM_ID_LASER_X_RED:
    case TEAM_ID_LASER_X_NEUTRAL:
        return RAD_PARSE_STATE_VALID;
    default:
        return RAD_PARSE_STATE_VALID;
    }
}
