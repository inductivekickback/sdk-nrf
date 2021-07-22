/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef TIMESLOT_H__
#define TIMESLOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TS_TIMEOUT_LEN_US    1000000
#define TS_SAFETY_MARGIN_US  100
#define TS_SKIPPED_TOLERANCE 10

#define TIMESLOT_CALLS_RADIO_IRQHANDLER 0

struct timeslot_config {
    /**
     * High frequency clock source, see MPSL_TIMESLOT_HFCLK_CFG.
     */
    uint8_t       hfclk;        

    /**
     * The timeslot length, see MPSL_TIMESLOT_LENGTH_MIN_US and MPSL_TIMESLOT_LENGTH_MAX_US.
     */
    uint32_t      length_us;    

    /**
     * Amount of time before a request times out.
     */
    uint32_t      timeout_us;
};

#define TS_DEFAULT_CONFIG { \
    .hfclk      = MPSL_TIMESLOT_HFCLK_CFG_XTAL_GUARANTEED, \
    .length_us  = 2000000, \
    .timeout_us = 2000000 \
}

struct timeslot_cb {
    /**
     * A (potentially unrecoverable) error has occurred.
     */
    void (*error)(int err);

    /**
     * Called at the beginning of every timeslot.
     */
    void (*start)(void);

    /**
     * Called TS_SAFETY_MARGIN_US before the end of every timeslot.
     */
    void (*end)(void);

    /**
     * A timeslot has been blocked or cancelled. The count parameter is set to the number
     * of consecutive timeslots that have been skipped.
     */
    void (*skipped)(uint8_t count);

    /**
     * The recurring timeslot has been stopped (the session is idle).
     */
    void (*stopped)(void);

#if !TIMESLOT_USE_RADIO_IRQHANDLER
    /**
     * Some like e.g. the ESB library defines its own RADIO_IRQHandler function. Setting
     * TIMESLOT_USE_RADIO_IRQHANDLER causes MPSL_TIMESLOT_SIGNAL_RADIO signals to call
     * RADIO_IRQHandler directly. Otherwise, this callback will be used whenever RADIO_IRQHandler
     * is called.
     */
    void (*radio_irq)(void);
#endif
};

/**
 * Opening a session is always the first step and there's no obvious
 * reason to ever close the session.
 */
int timeslot_open(struct timeslot_cb *cb); // TODO: Include config

/**
 * Request a recurring timeslot based on the given interval.
 */
int timeslot_start(uint32_t len_us, uint32_t interval_us);

/**
 * Stop requesting the recurring timeslot.
 */
int timeslot_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMESLOT_H__ */

/** @} */
