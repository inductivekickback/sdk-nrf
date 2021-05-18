/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <nrfx_pwm.h>

#include <drivers/rad_rx.h>
#include <drivers/rad_tx.h>

#define REFRESH_COUNT_28US 0

#define SPACE RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0, \
              RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0, \
              RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0, \
              RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0,RAD_TX_DUTY_CYCLE_0

#define MARK_0 RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50

#define MARK_1 MARK_0,MARK_0,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50, \
               RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50

#define PRE    MARK_1,MARK_1,MARK_1,MARK_1,RAD_TX_DUTY_CYCLE_50,RAD_TX_DUTY_CYCLE_50
#define BIT_0  SPACE,MARK_0
#define BIT_1  SPACE,MARK_1
#define PREFIX PRE,BIT_0,BIT_1,BIT_0,BIT_1,BIT_0,BIT_0
#define END    RAD_TX_DUTY_CYCLE_0

LOG_MODULE_REGISTER(rad_message_type_laser_x, CONFIG_RAD_MSG_TYPE_LASER_X_LOG_LEVEL);

#if CONFIG_RAD_RX_ACCEPT_LASER_X
rad_parse_state_t rad_msg_type_laser_x_parse(uint32_t          *message,
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
        if (IS_VALID_BIT_PULSE(message[i], RAD_RX_MSG_TYPE_LASER_X_SPACE_BIT_LEN_US)) {
            if (IS_VALID_BIT_PULSE(message[i+1], RAD_RX_MSG_TYPE_LASER_X_0_BIT_LEN_US)) {
                // This is a zero bit.
                continue;
            } else if (IS_VALID_BIT_PULSE(message[i+1], RAD_RX_MSG_TYPE_LASER_X_1_BIT_LEN_US)) {
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
#endif /* CONFIG_RAD_RX_ACCEPT_LASER_X */

#if CONFIG_RAD_TX_LASER_X
/*
 * NOTE: These values must be in RAM due to EasyDMA limitations.
 */
static uint16_t BLUE[]    = {PREFIX,BIT_0,BIT_1,END};
static uint16_t RED[]     = {PREFIX,BIT_1,BIT_0,END};
static uint16_t NEUTRAL[] = {PREFIX,BIT_1,BIT_1,END};

int rad_msg_type_laser_x_encode(team_id_laser_x_t team_id,
                               uint32_t          *refresh_count,
                               const uint16_t   **buf,
                               uint32_t          *len)
{
    switch (team_id) {
    case TEAM_ID_LASER_X_RED:
        *buf = &RED[0];
        *len = NRF_PWM_VALUES_LENGTH(RED);
        break;
    case TEAM_ID_LASER_X_BLUE:
        *buf = &BLUE[0];
        *len = NRF_PWM_VALUES_LENGTH(BLUE);
        break;
    case TEAM_ID_LASER_X_NEUTRAL:
        *buf = &NEUTRAL[0];
        *len = NRF_PWM_VALUES_LENGTH(NEUTRAL);
        break;
    default:
        return -1;
    }
    *refresh_count = REFRESH_COUNT_28US;
    return 0;
}

#endif /* RAD_TX_LASER_X */
