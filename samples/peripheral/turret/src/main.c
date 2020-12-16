/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <logging/log.h>

#include <dk_buttons_and_leds.h>

#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;
	LOG_INF("button_changed: %x", buttons);
}

static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

void main(void)
{
	k_sleep(K_MSEC(500));

	configure_gpio();

	LOG_INF("Turret started.");

	while(1) {
		dk_set_led_on(DK_LED1);
		k_sleep(K_MSEC(100));
 		dk_set_led_off(DK_LED1);
		k_sleep(K_MSEC(100));
	}
}
