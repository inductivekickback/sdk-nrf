/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef PROPRIETARY_RF_H__
#define PROPRIETARY_RF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TIMESLOT_USE_RADIO_IRQHANDLER 0

struct timeslot_cb {
    void (*error)(int err);
    void (*started)(void);
    void (*stopped)(void);
#if !TIMESLOT_USE_RADIO_IRQHANDLER
    void (*radio_irq)(void);
#endif
};

/**
 * Opening a session is always the first step and there's no obvious
 * reason to ever close the session.
 */
int proprietary_rf_session_open(struct timeslot_cb *cb);
int proprietary_rf_timeslot_start(uint16_t interval_ms);
int proprietary_rf_timeslot_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* PROPRIETARY_RF_H__ */

/** @} */
