/**
 * @file rad_rx.h
 *
 * @brief Public API for the Rad receiver driver
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_RX_H_
#define ZEPHYR_INCLUDE_RAD_RX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>
#include <stdbool.h>

typedef int (*rad_rx_init_t) (const struct device *dev);

/**
 * @brief Rad receiver driver API
 */
struct rad_rx_driver_api {
	rad_rx_init_t init;
};

static inline int rad_rx_init(const struct device *dev)
{
	struct rad_rx_driver_api *api;

	if (dev == NULL) {
		return -EINVAL;
	}

	api = (struct rad_rx_driver_api*)dev->api;

	if (api->init == NULL) {
		return -ENOTSUP;
	}
	return api->init(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_RX_H_ */
