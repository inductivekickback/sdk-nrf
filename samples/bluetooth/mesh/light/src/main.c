/*
 * Copyright (c) 2020 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <device.h>
#include <drivers/gpio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"


static const char * const   m_usb_detect_port  = DT_GPIO_LABEL(DT_NODELABEL(usb_detect), gpios);
static const u8_t           m_usb_detect_pin   = DT_GPIO_PIN(DT_NODELABEL(usb_detect),   gpios);
static const u32_t          m_usb_detect_flags = DT_GPIO_FLAGS(DT_NODELABEL(usb_detect), gpios);
static struct device       *m_usb_detect_dev;

static const char * const   m_button_port  = DT_GPIO_LABEL(DT_NODELABEL(button0), gpios);
static const u8_t           m_button_pin   = DT_GPIO_PIN(DT_NODELABEL(button0),   gpios);
static const u32_t          m_button_flags = DT_GPIO_FLAGS(DT_NODELABEL(button0), gpios);
static struct device       *m_button_dev;

static struct gpio_callback m_gpio_cb_data;

static const char * const   m_led_port  = DT_GPIO_LABEL(DT_NODELABEL(led0), gpios);
static const u8_t           m_led_pin   = DT_GPIO_PIN(DT_NODELABEL(led0),   gpios);
static const u32_t          m_led_flags = DT_GPIO_FLAGS(DT_NODELABEL(led0), gpios);
static struct device       *m_led_dev;


static void input_changed(struct device *dev, struct gpio_callback *cb, u32_t pins)
{
    /* The pins variable is a mask that describes pins in the form (1<<PIN_NUMBER). */
    int val;

    if (dev == m_usb_detect_dev && (pins & BIT(m_usb_detect_pin))) {
        val = gpio_pin_get(dev, m_usb_detect_pin);
        if (val) {
            printk("USB_DETECT high");
        } else {
            printk("USB_DETECT low");
        }
    }

    if (dev == m_button_dev && (pins & BIT(m_button_pin))) {
        val = gpio_pin_get(dev, m_usb_detect_pin);
        if (val) {
            printk("Button high");
        } else {
            printk("Button low");
        }
    }
}

static int gpio_init(void)
{
	int ret;

	m_usb_detect_dev = device_get_binding(m_usb_detect_port);
	if (!m_usb_detect_dev) {
		return -ENODEV;
	}

    m_button_dev = device_get_binding(m_button_port);
    if (!m_button_dev) {
        return -ENODEV;
    }

    m_led_dev = device_get_binding(m_led_port);
    if (!m_button_dev) {
        return -ENODEV;
    }

	ret = gpio_pin_configure(m_usb_detect_dev, m_usb_detect_pin, (GPIO_INPUT | m_usb_detect_flags));
	if (ret != 0) {
		return ret;
	}

    ret = gpio_pin_configure(m_button_dev, m_button_pin, (GPIO_INPUT | m_button_flags));
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_configure(m_led_dev, m_led_pin, (GPIO_OUTPUT | m_led_flags));
    if (ret != 0) {
        return ret;
    }

	ret = gpio_pin_interrupt_configure(m_usb_detect_dev,
                                       m_usb_detect_pin,
                                       GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		return ret;
	}

    ret = gpio_pin_interrupt_configure(m_button_dev,
                                       m_button_pin,
                                       GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        return ret;
    }

    u32_t pin_mask = (BIT(m_usb_detect_pin) | BIT(m_button_pin));
	gpio_init_callback(&m_gpio_cb_data, input_changed, pin_mask);

    gpio_add_callback(m_usb_detect_dev, &m_gpio_cb_data);
    gpio_add_callback(m_button_dev,     &m_gpio_cb_data);

    return 0;
}

static void bt_ready(int err)
{
    if (err) {
        return;
    }

    dk_leds_init();
    dk_buttons_init(NULL);

    err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
    if (err) {
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

int main(void)
{
	int err;

    err = gpio_init();
    if (err) {
        goto err_exit;
    }

	err = bt_enable(bt_ready);
	if (err) {
        goto err_exit;
	}

err_exit:
    /* TODO: Put the system into a safe state and then go to system_off? */
    return err;
}
