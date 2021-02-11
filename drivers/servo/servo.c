/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

// TODO: Use #ifdef to build an array of enabled PWM instances to choose from?

#define DT_DRV_COMPAT nordic_servo

#include <kernel.h>
#include <device.h>
#include <drivers/servo.h>
#include <devicetree.h>

#include <nrfx_pwm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(nordic_servo, CONFIG_NORDIC_SERVO_LOG_LEVEL);

/* If this flag is not set in the sequence values that are passed to
   nrf_drv_pwm then the polarity of the PWM waveform will be inverted. */
#define POLARITY_BIT  (0x8000UL)

#define MIN_VALUE     (600UL)
#define MAX_VALUE     (2500UL)
#define NEUTRAL_VALUE (((MAX_VALUE - MIN_VALUE) / 2) + MIN_VALUE)

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
    struct k_mutex mutex;
    bool ready;
} servo_group_t;

struct nordic_servo_data {
	uint8_t value;
	bool    ready;
};

struct nordic_servo_cfg {
    uint32_t pin;
    uint8_t  pwm_index;
    uint8_t  pwm_channel;
    uint8_t  init_value;
};

static servo_group_t m_available_pwms[] = {
#if CONFIG_NORDIC_SERVO_ALLOW_PWM0
	{
		.pwm_instance = NRFX_PWM_INSTANCE(0);
        .ready        = false;
    },
#endif
#if CONFIG_NORDIC_SERVO_ALLOW_PWM1
	{
		.pwm_instance = NRFX_PWM_INSTANCE(1);
        .ready        = false;
    },
#endif
#if CONFIG_NORDIC_SERVO_ALLOW_PWM2
	{
		.pwm_instance = NRFX_PWM_INSTANCE(2);
        .ready        = false;
    },
#endif
#if CONFIG_NORDIC_SERVO_ALLOW_PWM3
	{
		.pwm_instance = NRFX_PWM_INSTANCE(3);
        .ready        = false;
    },
#endif
};

#define NUM_AVAIL_PWMS (sizeof(m_available_pwms) / sizeof(servo_group_t))

static int m_channel_get(nrf_pwm_values_individual_t *p_values, uint8_t channel, uint8_t *p_value)
{
	uint16_t *p_channel;

    switch (channel) {
    case 0:
    	p_channel = &p_values->channel_0;
    	break;
    case 1:
    	p_channel = &p_values->channel_1;
    	break;
    case 2:
    	p_channel = &p_values->channel_2;
    	break;
    case 3:
    	p_channel = &p_values->channel_3;
    	break;
    default:
    	return -EINVAL;
    }

    *p_value = (uint8_t)PAM(*p_channel & ~POLARITY_BIT);
    return 0;
}

static int m_channel_set(nrf_pwm_values_individual_t *p_values, uint8_t channel, uint8_t value)
{
	uint16_t *p_channel;

	if (MAX_VALUE < value)
	{
		return -EINVAL;
	}

    switch (channel) {
    case 0:
    	p_channel = &p_values->channel_0;
    	break;
    case 1:
    	p_channel = &p_values->channel_1;
    	break;
    case 2:
    	p_channel = &p_values->channel_2;
    	break;
    case 3:
    	p_channel = &p_values->channel_3;
    	break;
    default:
    	return -EINVAL;
    }

    *p_channel = (MAP(value)|POLARITY_BIT);
    return 0;
}

static int m_nordic_servo_init(const struct device *dev)
{
    int           err;
    nrfx_err_t    nrfx_err;

    const struct nordic_servo_cfg *p_cfg  = dev->config;
    struct nordic_servo_data      *p_data = dev->data;

    if (p_data->ready) {
        /* Already initialized */
        return 0;
    }

    if (NUM_AVAIL_PWMS <= p_cfg->pwm_index) {
    	goto ERR_EXIT;
    }

    if (!m_available_pwms[p_cfg->pwm_index].ready) {

    	// TODO: Init the instance.

	    err = k_mutex_init(&m_available_pwms[p_cfg->pwm_index].mutex);
	    if (0 != err) {
	        return err;
	    }

    	m_available_pwms[p_cfg->pwm_index].ready = true;
    }

    // TODO: Write intitial value to correct pwm register.
    err = m_channel_set(&m_available_pwms[p_cfg->pwm_index].pwm_values,
                         p_cfg->pwm_channel,
                         p_cfg->init_value);
    if (0 != err) {
    	goto ERR_EXIT;
    }

    p_data->value = p_cfg->init_value;
    p_data->ready = true;

    return 0;

ERR_EXIT:
    return -ENXIO;
}

static int m_nordic_servo_write(const struct device *dev, uint8_t value)
{
    struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    p_data->value = value;

    // TODO: Actually write it.

    return 0;
}

static int m_nordic_servo_read(const struct device *dev, uint8_t *value)
{
    const struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    *value = p_data->value;

    return 0;
}

static int m_nordic_servo_start(const struct device *dev)
{
    const struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    // TODO: Can this be called on individual channels?

    return 0;
}

static int m_nordic_servo_stop(const struct device *dev)
{
    const struct nordic_servo_data *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_WRN("Device is not initialized yet");
        return -EBUSY;
    }

    // TODO: Can this be called on individual channels?

    return 0;
}

static const struct servo_driver_api m_nordic_servo_driver_api = {
    .init  = m_nordic_servo_init,
    .write = m_nordic_servo_write,
    .read  = m_nordic_servo_read,
    .start = m_nordic_servo_start,
    .stop  = m_nordic_servo_stop
};

#define INST(num) DT_INST(num, nordic_servo)

#define NORDIC_SERVO_DEVICE(n) \
    static const struct nordic_servo_cfg nordic_servo_cfg_##n = { \
        .pin = DT_PROP(INST(n), pin), \
        .init_value = DT_PROP(INST(n), init_value), \
        .pwm_channel = (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) % NRF_PWM_CHANNEL_COUNT), \
        .pwm_index = (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) / NRF_PWM_CHANNEL_COUNT) \
    }; \
    static struct nordic_servo_data nordic_servo_data_##n; \
    DEVICE_AND_API_INIT(nordic_servo_##n, \
                DT_LABEL(INST(n)), \
                m_nordic_servo_init, \
                &nordic_servo_data_##n, \
                &nordic_servo_cfg_##n, \
                POST_KERNEL, \
                CONFIG_NORDIC_SERVO_INIT_PRIORITY, \
                &m_nordic_servo_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NORDIC_SERVO_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Nordic servo driver enabled without any devices"
#endif
