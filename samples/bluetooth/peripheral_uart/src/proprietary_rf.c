/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

// TODO: Decide what timeslot length to use.
//       Use the current conn_interval_ms to decide basis.
//       Use GPIO to measure
//       Need a thread for firing up ESB and also performing started and stopped callbacks.

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

#define TS_LEN_US           2000
#define TS_TIMEOUT_LEN_US   1000000
#define TS_SAFETY_MARGIN_US 100

static uint16_t conn_interval_ms;
static uint8_t  blocked_canceled_count;
static bool     session_open;
static bool     timeslot_started;
static bool     timeslot_stopping;

static mpsl_timeslot_session_id_t mpsl_session_id;

/* NOTE: MPSL return params must be in static scope. */
static mpsl_timeslot_request_t request_earliest = {
	.request_type = MPSL_TIMESLOT_REQ_TYPE_EARLIEST,
	.params.earliest = {
	    .hfclk      = MPSL_TIMESLOT_HFCLK_CFG_XTAL_GUARANTEED,
		.priority   = MPSL_TIMESLOT_PRIORITY_NORMAL,
		.length_us  = TS_LEN_US,
		.timeout_us = TS_TIMEOUT_LEN_US
	}
};

static mpsl_timeslot_request_t request_normal = {
	.request_type = MPSL_TIMESLOT_REQ_TYPE_NORMAL,
	.params.normal = {
	    .hfclk       = MPSL_TIMESLOT_HFCLK_CFG_XTAL_GUARANTEED,
		.priority    = MPSL_TIMESLOT_PRIORITY_NORMAL,
		.length_us   = TS_LEN_US,
		.distance_us = 0
	}
};

static mpsl_timeslot_signal_return_param_t action_none = {
	.callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_NONE
};

static mpsl_timeslot_signal_return_param_t action_end = {
	.callback_action = MPSL_TIMESLOT_SIGNAL_ACTION_END
};

static mpsl_timeslot_signal_return_param_t action_request_normal = {
	.callback_action       = MPSL_TIMESLOT_SIGNAL_ACTION_REQUEST,
	.params.request.p_next = &request_normal
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
        NRF_TIMER0->TASKS_STOP          = 1;
    	NRF_TIMER0->TASKS_CLEAR         = 1;
    	NRF_TIMER0->MODE                = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
    	NRF_TIMER0->EVENTS_COMPARE[0]   = 0;
    	NRF_TIMER0->INTENSET            = (TIMER_INTENSET_COMPARE0_Set <<
    		                                   TIMER_INTENSET_COMPARE0_Pos);
    	NRF_TIMER0->CC[0]               = (TS_LEN_US - TS_SAFETY_MARGIN_US);
    	NRF_TIMER0->BITMODE             = (TIMER_BITMODE_BITMODE_24Bit <<
    					                       TIMER_BITMODE_BITMODE_Pos);
    	NRF_TIMER0->TASKS_START         = 1;
    	NVIC_EnableIRQ(TIMER0_IRQn);

    	// TODO: Power up the RADIO if necessary.
    	// TODO: Give a callback to the protocol to know that a timeslot has started.
 		break;
 	case MPSL_TIMESLOT_SIGNAL_TIMER0:
 		nrf_gpio_pin_toggle(TIMESLOT_PIN);
 		// TODO: Give a callback to the protocl to know that a timeslot is finished.
    	if (timeslot_stopping) {
    		return &action_end;
    	} else {
    		return &action_request_normal;
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
 		// TODO: Give a callback to the protocol to know that a timeslot was skipped.
 		blocked_canceled_count++;
    	if (timeslot_stopping) {
    		return &action_end;
    	}
    	request_normal.params.normal.distance_us = (conn_interval_ms*1000*(blocked_canceled_count+1));
    	return &action_request_normal;
 	case MPSL_TIMESLOT_SIGNAL_CANCELLED:
 		// TODO: Give a callback to the protocol to konw that a timeslot was skipped.
 		blocked_canceled_count++;
    	if (timeslot_stopping) {
    		return &action_end;
    	}
    	request_normal.params.normal.distance_us = (conn_interval_ms*1000*(blocked_canceled_count+1));
    	return &action_request_normal;
 	case MPSL_TIMESLOT_SIGNAL_SESSION_IDLE:
 		if (timeslot_stopping) {
 			timeslot_stopping = false;
 			timeslot_started  = false;

 			// TODO: If conn_interval_ms changed then re-request.
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

int proprietary_rf_timeslot_stop(void) {
	// TODO: This is async, use callback to notify.
	if (!session_open || !timeslot_started || timeslot_stopping) {
		return -1;
	}
    timeslot_stopping = true;
    return 0;
}

int proprietary_rf_timeslot_start(uint16_t interval) {
	// TODO: This is async, use callback to notify.
	if (!session_open || timeslot_started || timeslot_stopping) {
		return -1;
	}

	request_normal.params.normal.distance_us = (interval * 1000);
	conn_interval_ms                         = interval;
	blocked_canceled_count                   = 0;
	timeslot_started                         = true;

	return mpsl_timeslot_request(mpsl_session_id, &request_earliest);
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
