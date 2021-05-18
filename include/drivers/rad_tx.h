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

#if CONFIG_RAD_MSG_TYPE_LASER_X
#if CONFIG_RAD_TX_LASER_X
int rad_msg_type_laser_x_encode(team_id_laser_x_t team_id,
                                uint32_t         *refresh_count,
                                const uint16_t  **buf,
                                uint32_t         *len);
#endif /* CONFIG_RAD_TX_LASER_X */
#endif /* CONFIG_RAD_MSG_TYPE_LASER_X */

#define RAD_TX_MSG_TYPE_RAD_MSG_LEN     37
#define RAD_TX_MSG_TYPE_DYNASTY_MSG_LEN 37
#define RAD_TX_MSG_TYPE_LASER_X_MSG_LEN 37

#define RAD_TX_MSG_MAX_LEN              0

#if CONFIG_RAD_TX_RAD
#if RAD_TX_MSG_MAX_LEN < RAD_TX_MSG_TYPE_RAD_MSG_LEN
#undef RAD_TX_MSG_MAX_LEN
#define RAD_TX_MSG_MAX_LEN RAD_TX_MSG_TYPE_RAD_MSG_LEN
#endif
#endif /* CONFIG_RAD_TX_RAD */

#if CONFIG_RAD_TX_DYNASTY
#if RAD_TX_MSG_MAX_LEN < RAD_TX_MSG_TYPE_DYNASTY_MSG_LEN
#undef RAD_TX_MSG_MAX_LEN
#define RAD_TX_MSG_MAX_LEN RAD_TX_MSG_TYPE_DYNASTY_MSG_LEN
#endif
#endif /* CONFIG_RAD_TX_DYNASTY */

#if CONFIG_RAD_TX_LASER_X
#if RAD_TX_MSG_MAX_LEN < RAD_TX_MSG_TYPE_LASER_X_MSG_LEN
#undef RAD_TX_MSG_MAX_LEN
#define RAD_TX_MSG_MAX_LEN RAD_TX_MSG_TYPE_LASER_X_MSG_LEN
#endif
#endif /* CONFIG_RAD_TX_LASER_X */

#if CONFIG_RAD_TX
#if RAD_TX_MSG_MAX_LEN == 0
#error No Rad TX message types enabled
#endif
#endif

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
