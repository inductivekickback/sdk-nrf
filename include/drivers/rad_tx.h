/**
 * @file rad_tx.h
 *
 * @brief Public API for the Rad transmitter driver
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_TX_H_
#define ZEPHYR_INCLUDE_RAD_TX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>
#include <stdbool.h>

#include <rad.h>

/**
 * With a 16MHz clock the 37.9KHz period is ~422 ticks.
 */
#define RAD_TX_TICKS_PER_PERIOD     422
#define RAD_TX_DUTY_CYCLE_0         RAD_TX_TICKS_PER_PERIOD
#define RAD_TX_DUTY_CYCLE_50        (RAD_TX_TICKS_PER_PERIOD / 2)

typedef int (*rad_tx_init_t) (const struct device *dev);
typedef int (*rad_tx_blast_t)(const struct device *dev,
                                uint32_t refresh_count,
                                const uint16_t *data,
                                uint32_t len);

/**
 * @brief Rad transmitter driver API
 */
struct rad_tx_driver_api {
    rad_tx_init_t  init;
    rad_tx_blast_t blast;
};

static inline int rad_tx_init(const struct device *dev)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->init == NULL) {
        return -ENOTSUP;
    }
    return api->init(dev);
}

static inline int rad_tx_blast(const struct device *dev,
                               uint32_t refresh_count,
                               const uint16_t *data,
                               uint32_t len)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->blast == NULL) {
        return -ENOTSUP;
    }
    return api->blast(dev, refresh_count, data, len);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_TX_H_ */
