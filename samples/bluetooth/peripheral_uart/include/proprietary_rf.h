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

int proprietary_rf_session_open(void);

int proprietary_rf_timeslot_stop(void);

int proprietary_rf_timeslot_start(uint16_t interval);

#ifdef __cplusplus
}
#endif

#endif /* PROPRIETARY_RF_H__ */

/** @} */
