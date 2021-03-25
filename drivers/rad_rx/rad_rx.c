/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#define DT_DRV_COMPAT dmv_rad_rx

#include <kernel.h>
#include <device.h>
#include <drivers/rad_rx.h>
#include <devicetree.h>

#include <hal/nrf_gpio.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(rad_rx, CONFIG_RAD_RX_LOG_LEVEL);

struct rad_rx_data {
	bool ready;
};

struct rad_rx_cfg {
    const char * const port;
    const uint8_t      pin;
};

static int dmv_rad_rx_init(const struct device *dev)
{
    int err;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static const struct rad_rx_driver_api rad_rx_driver_api = {
    .init  = dmv_rad_rx_init,
};

#define INST(num) DT_INST(num, dmv_rad_rx)

#define RAD_RX_DEVICE(n) \
    static const struct rad_rx_cfg rad_rx_cfg_##n = { \
        .port  = DT_GPIO_LABEL(INST(n), gpios), \
        .pin   = DT_GPIO_PIN(INST(n),   gpios), \
    }; \
    static struct rad_rx_data rad_rx_data_##n; \
    DEVICE_DEFINE(rad_rx_##n, \
                DT_LABEL(INST(n)), \
                dmv_rad_rx_init, \
                NULL, \
                &rad_rx_data_##n, \
                &rad_rx_cfg_##n, \
                POST_KERNEL, \
                CONFIG_RAD_RX_INIT_PRIORITY, \
                &rad_rx_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAD_RX_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Rad laser tag receiver driver enabled without any devices"
#endif
