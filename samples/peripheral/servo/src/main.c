/*
 * Copyright (c) 2020 Daniel Veilleux
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <string.h>
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <sys/__assert.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void main(void)
{
    int ret;
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
            LOG_INF("Found device: %s", dev->name);
        }
        dev++;
    }

    dev = device_get_binding("SERVO_0");
    if (dev == NULL) {
        LOG_ERR("Failed to get dev binding");
        return;
    }
    LOG_INF("dev is %p, name is %s", dev, dev->name);

    while (1) {
    	k_sleep(K_MSEC(100));
    }
    LOG_INF("exiting");
}
