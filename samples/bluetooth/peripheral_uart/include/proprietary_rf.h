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

#include <esb.h>

typedef void (*proprietary_rf_cb)(struct esb_payload *tx_payload);

int proprietary_rf_init(proprietary_rf_cb p_cb);

void proprietary_rf_end(void);

void proprietary_rf_skipped(uint8_t count);

void proprietary_rf_start(void);

#ifdef __cplusplus
}
#endif

#endif /* PROPRIETARY_RF_H__ */

/** @} */
