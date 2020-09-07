/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @brief Nordic Mesh light sample
 */
#include <device.h>
#include <drivers/gpio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"

struct gpio_pin {
	const char * const port;
	const u8_t number;
};

static const struct gpio_pin usb_detect_pin = {
	DT_GPIO_LABEL(DT_NODELABEL(usbdetect), gpios),
	DT_GPIO_PIN(DT_NODELABEL(usbdetect), gpios)
};

static struct device *usb_detect_button;
static struct gpio_callback button_cb_data;

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

static void button_pressed(struct device *dev, struct gpio_callback *cb, u32_t pins)
{
    int val;

    val = gpio_pin_get_raw(usb_detect_button, usb_detect_pin.number);
    if (val) {
        printk("Button high");
    } else {
        printk("Button low");
    }
}

static int button_init(void)
{
	int ret;

	usb_detect_button = device_get_binding(usb_detect_pin.port);
	if (!usb_detect_button) {
		return -ENODEV;
	}

	ret = gpio_pin_configure(usb_detect_button, usb_detect_pin.number, GPIO_INPUT);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_interrupt_configure(usb_detect_button,
                                       usb_detect_pin.number,
                                       GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(usb_detect_pin.number));
	gpio_add_callback(usb_detect_button, &button_cb_data);
    return 0;
}

void main(void)
{
	int err;

	printk("Initializing...\n");

    err = button_init();
    if (err) {
        printk("Failed to initialize buttons");
    }

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}
