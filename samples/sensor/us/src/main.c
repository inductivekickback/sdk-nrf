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

static int measure(const struct device *dev)
{
    int ret;
    struct sensor_value distance;

    ret = sensor_sample_fetch_chan(dev, SENSOR_CHAN_ALL);
    switch (ret) {
    case 0:
        ret = sensor_channel_get(dev, SENSOR_CHAN_DISTANCE, &distance);
        if (ret) {
            printk("sensor_channel_get failed ret %d", ret);
            return ret;
        }
        printk("%s: %d.%02dM", dev->name, (distance.val1 / 1000000), (distance.val2 / 10000));
        break;
    case -EIO:
        printk("%s: Could not read device", dev->name);
        break;
    default:
        printk("Error when reading device: %s", dev->name);
        break;
    }
    return 0;
}

void main(void)
{
    int ret;
	const struct device *dev;

	if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
		/* Give RTT log time to be flushed before executing tests */
		k_sleep(K_MSEC(500));
	}

    dev = device_get_binding("HC-SR04_0");
    if (dev == NULL) {
        printk("Failed to get dev binding");
        return;
    }

    ret = measure(dev);
    if (ret) {
        printk("Failed to fetch and get measurement");
    }
}
