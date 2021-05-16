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

rad_parse_state_t rad_msg_type_rad_parse(uint32_t      *message,
	                                     uint32_t       len,
	                                     rad_msg_rad_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by sixteen bits:
     *     START: Active pulse of ~1.58ms (60 periods @ 38KHz)
     *     0:     Inactive for ~0.39ms (15 periods) followed by an active pulse of ~0.39ms
     *     1:     Inactive for ~0.78ms (30 periods) followed by an active pulse of ~0.39ms
     */
	int i=1;

	msg->reserved = 0;
	for (int j=1; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->reserved |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->team_id = 0;
	for (int j=1; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->team_id |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->player_id = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->player_id |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->special = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->special |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->damage = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_MSG_ACTIVE_BIT_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_0_BIT_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->damage |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	return RAD_PARSE_STATE_VALID;
}
