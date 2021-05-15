/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <rad.h>

#define PISTOL_CHECKSUM_ADD	     5
#define SHOTGUN_CHECKSUM_ADD     6
#define SHOTGUN_IRR_CHECKSUM_ADD 7
#define ROCKET_CHECKSUM_ADD      7
#define ROCKET_IRR_CHECKSUM_ADD  8

#define COMMON_PREAMBLE         170

LOG_MODULE_REGISTER(rad_message_type_dynasty, CONFIG_RAD_MSG_TYPE_DYNASTY_LOG_LEVEL);

rad_parse_state_t rad_msg_type_dynasty_parse(uint32_t *message,
	                                             uint32_t len,
	                                             rad_msg_dynasty_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by a 16-bit preamble and then twenty-four bits:
     *     START:    Active pulse of ~1.66ms
     *     PREAMBLE: 0b0000000010101010
     *     0:        Inactive or active for ~0.4ms (can be as long as 0.58ms)
     *     1:        Inactive or active for ~0.75ms (can be as short as 0.61ms)
     *
     *     This blaster's pulse lengths are quite sloppy so parsing is relaxed.
     */
    int      i       =1;
    uint16_t preamble=0;

    for (int j=15; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            preamble |= (1<<j);
        }
    }

    if (COMMON_PREAMBLE != preamble) {
    	return RAD_PARSE_STATE_INVALID;
    }

    msg->team_id = 0;
    for (int j=7; j>=0; i++,j--) {
		if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->team_id |= (1<<j);
        }
    }

    switch (msg->team_id) {
    case TEAM_ID_DYNASTY_BLUE:
    case TEAM_ID_DYNASTY_RED:
    case TEAM_ID_DYNASTY_GREEN:
    case TEAM_ID_DYNASTY_WHITE:
        break;
    default:
        return RAD_PARSE_STATE_INVALID;
    }

    msg->weapon_id = 0;
    for (int j=7; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->weapon_id |= (1<<j);
        }
    }

    msg->checksum = 0;
    for (int j=7; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_MSG_1_BIT_LEN_US)) {
            // This is a one bit.
            msg->checksum |= (1<<j);
        }
    }

    switch (msg->weapon_id) {
    case WEAPON_ID_DYNASTY_PISTOL:
    	if (msg->checksum != (msg->team_id + PISTOL_CHECKSUM_ADD)) {
    		return RAD_PARSE_STATE_INVALID;
    	}
    	break;
    case WEAPON_ID_DYNASTY_SHOTGUN_SMG:
    	switch (msg->team_id) {
		case TEAM_ID_DYNASTY_BLUE:
		case TEAM_ID_DYNASTY_RED:
		case TEAM_ID_DYNASTY_GREEN:    		
        	if (msg->checksum != (msg->team_id + SHOTGUN_CHECKSUM_ADD)) {
    	   		return RAD_PARSE_STATE_INVALID;
    	   	}
        	break;
        case TEAM_ID_DYNASTY_WHITE:
        	if (msg->checksum != (msg->team_id + SHOTGUN_IRR_CHECKSUM_ADD)) {
    	   		return RAD_PARSE_STATE_INVALID;
    	   	}
        	break;
        default:
        	return RAD_PARSE_STATE_INVALID;
        }
        break;
    case WEAPON_ID_DYNASTY_ROCKET:
    	switch (msg->team_id) {
    	case TEAM_ID_DYNASTY_BLUE:
        case TEAM_ID_DYNASTY_RED:
   	       	if (msg->checksum != (msg->team_id + ROCKET_CHECKSUM_ADD)) {
    			return RAD_PARSE_STATE_INVALID;
	    	}
        	break;
        case TEAM_ID_DYNASTY_GREEN:
        case TEAM_ID_DYNASTY_WHITE:
           	if (msg->checksum != (msg->team_id + ROCKET_IRR_CHECKSUM_ADD)) {
	    		return RAD_PARSE_STATE_INVALID;
    		}
        	break;
        default:
        	return RAD_PARSE_STATE_INVALID;
    	}
        break;
    default:
    	return RAD_PARSE_STATE_INVALID;
    }

    return RAD_PARSE_STATE_VALID;
}
