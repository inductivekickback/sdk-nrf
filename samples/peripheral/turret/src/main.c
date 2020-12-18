/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <logging/log.h>

#include <nrfx_pwm.h>
#include <dk_buttons_and_leds.h>

#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


static void m_button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;
	LOG_INF("button_changed: %x", buttons);
}

static void m_configure_gpio(void)
{
	int err;

	err = dk_buttons_init(m_button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

static void m_pwm_handler(nrfx_pwm_evt_type_t event_type, void * p_context)
{
	LOG_INF("PWM event: %d", event_type);
}

static nrfx_pwm_t m_pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_common_t m_pwm_values[5];

static nrfx_err_t m_pwm_config(void)
{
	nrfx_err_t nrfx_err;

	nrfx_pwm_config_t config = NRFX_PWM_DEFAULT_CONFIG(31,
												NRFX_PWM_PIN_NOT_USED,
												NRFX_PWM_PIN_NOT_USED,
												NRFX_PWM_PIN_NOT_USED);

#define PWM_CLOKC_FREQ   8000000
#define IR_FREQ          37900
#define TICKS_PER_PERIOD 211

	m_pwm_values[0] = TICKS_PER_PERIOD / 2;
	m_pwm_values[1] = TICKS_PER_PERIOD;
	m_pwm_values[2] = TICKS_PER_PERIOD / 2;
	m_pwm_values[3] = TICKS_PER_PERIOD / 2;
	m_pwm_values[4] = TICKS_PER_PERIOD;
//	m_pwm_values[0] = (TICKS_PER_PERIOD / 2);
//	m_pwm_values[1] = 0;
//	m_pwm_values[2] = TICKS_PER_PERIOD;

	config.base_clock = NRF_PWM_CLK_8MHz;
	config.top_value  = TICKS_PER_PERIOD;
	config.load_mode  = NRF_PWM_LOAD_COMMON;

    IRQ_CONNECT(DT_IRQN(DT_INST(0, nordic_nrf_pwm)),
            DT_IRQ(DT_INST(0, nordic_nrf_pwm), priority),
            nrfx_isr,
            nrfx_pwm_0_irq_handler,
            0);

	nrfx_err = nrfx_pwm_init(&m_pwm, &config, m_pwm_handler, NULL);
	if (NRFX_SUCCESS != nrfx_err)
	{
		return nrfx_err;
	}

	return nrfx_err;
}

static void m_blast(void)
{
    nrf_pwm_sequence_t const seq =
    {
        .values.p_common = &m_pwm_values[0],
        .length              = NRF_PWM_VALUES_LENGTH(m_pwm_values),
        .repeats             = 14,
        .end_delay           = 0
    };
    nrfx_pwm_simple_playback(&m_pwm,
                             &seq,
                             1,
                             NRFX_PWM_FLAG_STOP);
}

void main(void)
{
	nrfx_err_t nrfx_err;

	m_configure_gpio();
	nrfx_err = m_pwm_config();
	if (NRFX_SUCCESS != nrfx_err) {
		dk_set_led_on(DK_LED2);
	}

	for (int i=0; i < 5; i++) {
		m_blast();
		k_sleep(K_MSEC(100));
	}

	LOG_INF("Turret started.");

	while(1) {
		dk_set_led_on(DK_LED1);
		k_sleep(K_MSEC(100));
 		dk_set_led_off(DK_LED1);
		k_sleep(K_MSEC(100));
	}
}
