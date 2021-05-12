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

#include <rad.h>

/* This callback is called from the driver to notify the app that a message was received. */
typedef void (*rad_rx_callback_t) (rad_msg_type_t msg_type, void *data);

typedef int (*rad_rx_init_t)         (const struct device *dev);
typedef int (*rad_rx_set_callback_t) (const struct device *dev, rad_rx_callback_t *cb);

/**
 * @brief Rad receiver driver API
 */
struct rad_rx_driver_api {
	rad_rx_init_t         init;
	rad_rx_set_callback_t set_callback;
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

static inline int rad_rx_set_callback(const struct device *dev, rad_rx_callback_t *cb)
{
	struct rad_rx_driver_api *api;

	if (dev == NULL) {
		return -EINVAL;
	}

	api = (struct rad_rx_driver_api*)dev->api;

	if (api->init == NULL) {
		return -ENOTSUP;
	}
	return api->set_callback(dev, cb);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_RX_H_ */
