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
#include <string.h>


void main(void)
{
	const struct device *dev;

	if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
		/* Give RTT log time to be flushed before executing tests */
		k_sleep(K_MSEC(500));
	}

    size_t len = z_device_get_all_static(&dev);
    const struct device *dev_end = dev + len;
    while (dev < dev_end) {
        if (z_device_ready(dev)
            && (dev->name != NULL)
            && (strlen(dev->name) != 0)) {
            printk("Found device: %s\r\n", dev->name);
        }
        dev++;
    }

}
