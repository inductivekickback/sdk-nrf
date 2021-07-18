/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

// TODO: Decide what timeslot length to use.
//       Use the current conn_interval to decide basis.
//       Use GPIO to measure

#include <zephyr.h>
#include <stdio.h>

#include <logging/log.h>

#define LOG_MODULE_NAME proprietary_rf
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <mpsl.h>
#include <mpsl_timeslot.h>

#include <hal/nrf_gpio.h>
#define TIMESLOT_PIN 4

#include <proprietary_rf.h>

static uint16_t conn_interval;
static bool     session_open;
static bool     timeslot_started;
static bool     timeslot_stopping;

static mpsl_timeslot_session_id_t mpsl_session_id;

/* NOTE: MPSL return params must be in static scope. */
static mpsl_timeslot_signal_return_param_t action_none = {
	.callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_NONE
};

static mpsl_timeslot_signal_return_param_t action_end = {
	.callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_END
};

static mpsl_timeslot_signal_return_param_t*
mpsl_cb(mpsl_timeslot_session_id_t session_id, uint32_t signal)
{
	switch (signal) {
    case MPSL_TIMESLOT_SIGNAL_START:
    	nrf_gpio_pin_toggle(TIMESLOT_PIN);
    	if (timeslot_stopping) {
    		return &action_end;
    	}
    	// TODO: Configure TIMER0
 		break;
 	case MPSL_TIMESLOT_SIGNAL_TIMER0:
 		nrf_gpio_pin_toggle(TIMESLOT_PIN);
 		// TODO: Close the timeslot
    	if (timeslot_stopping) {
    		return &action_end;
    	}
 		break;
 	case MPSL_TIMESLOT_SIGNAL_RADIO:
 		// TODO: Forward to ESB 
 		break;
 	case MPSL_TIMESLOT_SIGNAL_EXTEND_FAILED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_EXTEND_SUCCEEDED:
 		break;
 	case MPSL_TIMESLOT_SIGNAL_BLOCKED:
 		// TODO: Re-request at next interval of offset with high priority
    	if (timeslot_stopping) {
    		return &action_end;
    	}
 		break;
 	case MPSL_TIMESLOT_SIGNAL_CANCELLED:
  		// TODO: Re-request at next interval of offset with high priority
    	if (timeslot_stopping) {
    		return &action_end;
    	}
 		break;
 	case MPSL_TIMESLOT_SIGNAL_SESSION_IDLE:
 		if (timeslot_stopping) {
 			timeslot_stopping = false;
 			timeslot_started  = false;

 			// TODO: If conn_interval changed then re-request.
 		} else {
 			LOG_ERR("MPSL_TIMESLOT_SIGNAL_SESSION_IDLE");
 		}
 		break;
 	case MPSL_TIMESLOT_SIGNAL_INVALID_RETURN:
 		LOG_ERR("MPSL_TIMESLOT_SIGNAL_INVALID_RETURN");
 		break;
 	case MPSL_TIMESLOT_SIGNAL_SESSION_CLOSED:
 		LOG_ERR("MPSL_TIMESLOT_SIGNAL_SESSION_CLOSED");
 		break;
 	case MPSL_TIMESLOT_SIGNAL_OVERSTAYED:
	 	LOG_ERR("MPSL_TIMESLOT_SIGNAL_OVERSTAYED");
 		break;
 	default:
 		break;
 	};

    return &action_none;
}

int proprietary_rf_timeslot_start(uint16_t interval) {
	if (!session_open || timeslot_started || timeslot_stopping) {
		return -1;
	}

	conn_interval = interval;
	// TODO: Request earliest.

	return 0;
}

int proprietary_rf_timeslot_stop(void) {
	if (!session_open || !timeslot_started || timeslot_stopping) {
		return -1;
	}
    timeslot_stopping = true;
    return 0;
}

int proprietary_rf_session_open(void) {
	if (session_open) {
		return -1;
	}

 	int err = mpsl_timeslot_session_open(mpsl_cb, &mpsl_session_id);
 	if (err) {
 		return err;
 	}

	nrf_gpio_cfg_output(TIMESLOT_PIN);
	nrf_gpio_pin_clear(TIMESLOT_PIN);

 	session_open = true;
 	return 0;
}
