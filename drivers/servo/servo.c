/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#define DT_DRV_COMPAT nordic_servo

#include <kernel.h>
#include <device.h>
#include <drivers/servo.h>
#include <devicetree.h>

#include <nrfx_pwm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(nordic_servo, CONFIG_NORDIC_SERVO_LOG_LEVEL);

#define NUM_PWM_PERIPHS 4

// If this flag is not set in the sequence values that are passed to
// nrf_drv_pwm then the polarity of the PWM waveform will be inverted.
// Inverted values are not useful to a servo so it is assumed that if
// a value in a sequence doesn't have this flag set then that channel
// is not enabled.
#define CH_ENABLED_MASK (0x8000UL)

#define MIN_VALUE     (600UL)
#define MAX_VALUE     (2500UL)
#define NEUTRAL_VALUE (((MAX_VALUE-MIN_VALUE)/2) + MIN_VALUE)

// Simple functions for mapping a value betwen [0, 100] to the
// range [MIN_VALUE, MAX_VALUE] and vice versa.
#define MAP(x) ((x) * (MAX_VALUE - MIN_VALUE)/100 + MIN_VALUE)
#define PAM(x) (((x) - MIN_VALUE) * 100 / (MAX_VALUE - MIN_VALUE))

// Structs of this type need to kept in the global portion (static) of RAM
// (not const) because they are accessed by EasyDMA.
typedef struct
{
    nrfx_pwm_t pwm_instance;
    nrf_pwm_values_individual_t pwm_values;
} servo_group_t;

static struct nordic_servo_shared_resources {
    struct k_mutex           mutex;
    servo_group_t            periphs[NUM_PWM_PERIPHS]; /* TODO: Pull this from DTSI or something? */
    bool                     ready; /* The module has been initialized */
} m_shared_resources;

struct nordic_servo_data {
	uint8_t value;
}

struct nordic_servo_cfg {
    uint32_t pin;
    uint8_t  pwm_indx;
    uint8_t  value_indx;
    uint8_t  init_value;
};

static int nordic_servo_init(const struct device *dev)
{
    int           err;
    nrfx_err_t    nrfx_err;

    const struct nordic_servo_cfg *p_cfg  = dev->config;
    struct nordic_servo_data      *p_data = dev->data;

    p_data->value = p_cfg->init_value;

    if (m_shared_resources.ready) {
        /* Already initialized */
        return 0;
    }

    err = k_mutex_init(&m_shared_resources.mutex);
    if (0 != err) {
        return err;
    }

    // TODO: Need to know if individual periph instances are init'd yet.

    m_shared_resources.ready = true;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static int nordic_servo_write(const struct device *dev, uint8_t value)
{
    const struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!m_shared_resources.ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    p_data.value = value;

    // TODO: Actually write it.

    return 0;
}

static int nordic_servo_read(const struct device *dev, uint8_t *value)
{
    const struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!m_shared_resources.ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    *value = p_data.value;

    return 0;
}

static const struct servo_driver_api nordic_servo_driver_api = {
    .init  = nordic_servo_init,
    .write = nordic_servo_write,
    .read  = nordic_servo_read,
};

#define INST(num) DT_INST(num, nordic_servo)

#define NORDIC_SERVO_DEVICE(n) \
    static const struct nordic_servo_cfg nordic_servo_cfg_##n = { \
        .pin = DT_PROP(INST(n), pin), \
        .init_value = DT_PROP(INST(n), init_value) \
        .value_indx = (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) % NUM_PWM_PERIPHS), \
        .pwm_indx = (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) / NUM_PWM_PERIPHS) \
    }; \
    static struct nordic_servo_data nordic_servo_data_##n; \
    DEVICE_AND_API_INIT(nordic_servo_##n, \
                DT_LABEL(INST(n)), \
                nordic_servo_init, \
                &nordic_servo_data_##n, \
                &nordic_servo_cfg_##n, \
                POST_KERNEL, \
                CONFIG_SERVO_INIT_PRIORITY, \
                &nordic_servo_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NORDIC_SERVO_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Nordic servo driver enabled without any devices"
#endif
