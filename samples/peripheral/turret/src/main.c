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

#define PWM_PIN 31

/*
 * With an 8MHz clock the 37.9KHz period is ~211 ticks.
 */
#define TICKS_PER_PERIOD 		211
#define DUTY_CYCLE_0     		TICKS_PER_PERIOD
#define DUTY_CYCLE_50    		(TICKS_PER_PERIOD / 2)
#define REFRESH_COUNT_400US    	14


/*
 * BLUE pistol, shotgun, rocket.
 * 0  0  0  0_ 0  0  0  0_ 0  0  0 _1 
 * 0  0  0  0_ 0  0  0  1  0  0  0 _1_
 * 0  0  0  0_ 0  0  0  1_ 0  0  1  0 
 */
#define PREAMBLE DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50
#define SH       DUTY_CYCLE_50
#define LH       DUTY_CYCLE_50,DUTY_CYCLE_50
#define SL       DUTY_CYCLE_0
#define LL       DUTY_CYCLE_0,DUTY_CYCLE_0
#define PREFIX   PREAMBLE,SL,SH,SL,SH,SL,SH,SL,SH,LL,SH,LL,SH,LL,SH,LL,SH,SL,SH,SL,SH,SL,SH
#define END      DUTY_CYCLE_0

/*
 * NOTE: These values must be in RAM due to EasyDMA limitations.
 */
static uint16_t BLUE_PISTOL_CMD[] = {PREFIX, END};

static nrfx_pwm_t m_pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_sequence_t m_seq = {
    .values.p_common = NULL,
    .length          = 0,
    .repeats         = REFRESH_COUNT_400US,
    .end_delay       = 0
};

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
	// TODO: Wait for the NRFX_PWM_EVT_STOPPED event before allowing next blast.
	LOG_INF("PWM event: %d", event_type);
}

static nrfx_err_t m_pwm_config(void)
{
	nrfx_pwm_config_t config = NRFX_PWM_DEFAULT_CONFIG(PWM_PIN,
														NRFX_PWM_PIN_NOT_USED,
														NRFX_PWM_PIN_NOT_USED,
														NRFX_PWM_PIN_NOT_USED);
	config.base_clock = NRF_PWM_CLK_8MHz;
	config.top_value  = TICKS_PER_PERIOD;
	config.load_mode  = NRF_PWM_LOAD_COMMON;

    IRQ_CONNECT(DT_IRQN(DT_INST(0, nordic_nrf_pwm)),
            DT_IRQ(DT_INST(0, nordic_nrf_pwm), priority),
            nrfx_isr,
            nrfx_pwm_0_irq_handler,
            0);
	return nrfx_pwm_init(&m_pwm, &config, m_pwm_handler, NULL);
}

static void m_blast(uint16_t *data, uint32_t len)
{
    m_seq.values.p_common = data;
    m_seq.length          = len;
    nrfx_pwm_simple_playback(&m_pwm,
                             &m_seq,
                             1,
                             NRFX_PWM_FLAG_STOP);
}

void main(void)
{
	nrfx_err_t nrfx_err;

	m_configure_gpio();

	nrfx_err = m_pwm_config();
	if (NRFX_SUCCESS != nrfx_err) {
		LOG_ERR("PWM config failed: %x", nrfx_err);
		dk_set_led_on(DK_LED2);
	}

	LOG_INF("Turret started.");

	while (true) {
		m_blast(&BLUE_PISTOL_CMD[0], NRF_PWM_VALUES_LENGTH(BLUE_PISTOL_CMD));
		dk_set_led_on(DK_LED1);
		k_sleep(K_MSEC(50));
 		dk_set_led_off(DK_LED1);
		k_sleep(K_MSEC(50));
	}
}
