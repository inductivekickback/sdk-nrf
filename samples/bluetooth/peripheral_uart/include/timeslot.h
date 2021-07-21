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

#define TIMESLOT_CALLS_RADIO_IRQHANDLER 0

struct timeslot_cb {
    /**
     * 
     */
    void (*error)(int err);

    /**
     * 
     */
    void (*start)(void);

    /**
     * 
     */
    void (*stop)(void);

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
int timeslot_open(struct timeslot_cb *cb);
int timeslot_start(uint16_t interval_ms);
int timeslot_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMESLOT_H__ */

/** @} */
