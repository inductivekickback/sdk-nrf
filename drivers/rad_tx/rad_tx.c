/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#define DT_DRV_COMPAT dmv_rad_tx

#include <kernel.h>
#include <device.h>
#include <drivers/rad_tx.h>
#include <devicetree.h>

#include <hal/nrf_gpio.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(rad_tx, CONFIG_RAD_TX_LOG_LEVEL);

struct rad_tx_data {
	bool ready;
};

struct rad_tx_cfg {
    const char * const port;
    const uint8_t      pin;
};

static int dmv_rad_tx_init(const struct device *dev)
{
    int err;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static const struct rad_tx_driver_api rad_tx_driver_api = {
    .init  = dmv_rad_tx_init,
};

#define INST(num) DT_INST(num, dmv_rad_tx)

#define RAD_TX_DEVICE(n) \
    static const struct rad_tx_cfg rad_tx_cfg_##n = { \
        .port  = DT_GPIO_LABEL(INST(n), gpios), \
        .pin   = DT_GPIO_PIN(INST(n),   gpios), \
    }; \
    static struct rad_tx_data rad_tx_data_##n; \
    DEVICE_DEFINE(rad_tx_##n, \
                DT_LABEL(INST(n)), \
                dmv_rad_tx_init, \
                NULL, \
                &rad_tx_data_##n, \
                &rad_tx_cfg_##n, \
                POST_KERNEL, \
                CONFIG_RAD_TX_INIT_PRIORITY, \
                &rad_tx_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAD_TX_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Rad laser tag transmitter driver enabled without any devices"
#endif
