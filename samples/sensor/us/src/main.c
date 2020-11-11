/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/__assert.h>


void main(void)
{
	const struct device *dev;

	if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
		/* Give RTT log time to be flushed before executing tests */
		k_sleep(K_MSEC(500));
	}
}
