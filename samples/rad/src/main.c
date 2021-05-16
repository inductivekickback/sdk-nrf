/*
 * Copyright (c) 2021 Daniel Veilleux
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

#include <drivers/rad_rx.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void rad_rx_cb(rad_msg_type_t msg_type, void *data)
{
    switch (msg_type) {
    case RAD_MSG_TYPE_LASER_X:
        {
            rad_msg_laser_x_t *msg = (rad_msg_laser_x_t*)data;
            switch (msg->team_id) {
            case TEAM_ID_LASER_X_BLUE:
                LOG_INF("Laser X team ID: BLUE");
                break;
            case TEAM_ID_LASER_X_RED:
                LOG_INF("Laser X team ID: RED");
                break;
            case TEAM_ID_LASER_X_NEUTRAL:
                LOG_INF("Laser X team ID: NEUTRAL");
                break;
            default:
                LOG_INF("Invalid Laser X team ID.");
                break;
            }
        }
        break;
    case RAD_MSG_TYPE_DYNASTY:
        {
            rad_msg_dynasty_t *msg = (rad_msg_dynasty_t*)data;
            LOG_INF("Dynasty team ID (%d), weapon ID (%d)", msg->team_id, msg->weapon_id);
        }
        break;
    case RAD_MSG_TYPE_RAD:
        LOG_INF("Rad message received");
        break;
    default:
        LOG_INF("Unhandled Rad message type (%d).", msg_type);
        break;
    }
}

void main(void)
{
    int ret;
    const struct device *dev;

    if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
        /* Give RTT log time to be flushed before executing tests */
        k_sleep(K_MSEC(500));
    }

    dev = device_get_binding("rad_rx0");

    if (dev == NULL) {
        LOG_ERR("Failed to get dev binding");
        return;
    }
    LOG_INF("dev is %p, name is %s", dev, dev->name);

    ret = rad_rx_set_callback(dev, rad_rx_cb);
    if (ret) {
        LOG_ERR("Failed to set rad rx callback");
        return;
    }

    while (1) {
    	k_sleep(K_MSEC(5));
    }
}
