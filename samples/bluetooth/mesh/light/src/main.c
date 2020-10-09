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
#include "model_handler.h"
#include "fogger.h"

static const char * const   m_button_port  = DT_GPIO_LABEL(DT_NODELABEL(button0), gpios);
static const u8_t           m_button_pin   = DT_GPIO_PIN(DT_NODELABEL(button0),   gpios);
static const u32_t          m_button_flags = DT_GPIO_FLAGS(DT_NODELABEL(button0), gpios);
static struct device       *m_button_dev;

static struct gpio_callback m_gpio_cb_data;

static const char * const   m_err_led_port  = DT_GPIO_LABEL(DT_NODELABEL(led2), gpios);
static const u8_t           m_err_led_pin   = DT_GPIO_PIN(DT_NODELABEL(led2),   gpios);
static const u32_t          m_err_led_flags = DT_GPIO_FLAGS(DT_NODELABEL(led2), gpios);
static struct device       *m_err_led_dev;

static bool m_button_pressed;

static void input_changed(struct device *dev, struct gpio_callback *cb, u32_t pins)
{
    /* The pins variable is a mask that describes pins in the form (1<<PIN_NUMBER). */
    int val;

    if (dev == m_button_dev && (pins & BIT(m_button_pin))) {
        val = gpio_pin_get(dev, m_button_pin);
        if (val) {
            m_button_pressed = true;
            fogger_start();
        } else {
            m_button_pressed = false;
            fogger_stop();
        }
    }
}

static int gpio_init(void)
{
	int ret;

    m_button_dev = device_get_binding(m_button_port);
    if (!m_button_dev) {
        return -ENODEV;
    }

    m_err_led_dev = device_get_binding(m_err_led_port);
    if (!m_err_led_dev) {
        return -ENODEV;
    }

    ret = gpio_pin_configure(m_button_dev, m_button_pin, (GPIO_INPUT | m_button_flags));
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_configure(m_err_led_dev, m_err_led_pin, (GPIO_OUTPUT | m_err_led_flags));
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_interrupt_configure(m_button_dev,
                                       m_button_pin,
                                       GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        return ret;
    }

	gpio_init_callback(&m_gpio_cb_data, input_changed, BIT(m_button_pin));
    gpio_add_callback(m_button_dev, &m_gpio_cb_data);

    return 0;
}

static void bt_ready(int err)
{
    if (err) {
        return;
    }

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

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf)
{
    /* TODO: Put the system into a safe state and then go to system_off? */
    gpio_pin_set(m_err_led_dev, m_err_led_pin, 1);
    k_fatal_halt(reason);
}

void fogger_callback(enum fogger_state status)
{
    // TODO: State needs to be forwarded to model handler.

    switch (status) {
    case FOGGER_STATE_HEATING:
        model_handler_elem_update(0, false);
        model_handler_elem_update(1, false);
        break;
    case FOGGER_STATE_READY:
        if (m_button_pressed) {
            fogger_start();
        }
        model_handler_elem_update(0, false);
        model_handler_elem_update(1, true);
        break;
    case FOGGER_STATE_FOGGING:
        model_handler_elem_update(0, true);
        model_handler_elem_update(1, true);
        break;
    default:
        k_oops();
        break;
    }
}

int main(void)
{
	int err;

    err = gpio_init();
    if (err) {
        k_oops();
    }

    m_button_pressed = gpio_pin_get(m_button_dev, m_button_pin);

	err = bt_enable(bt_ready);
	if (err) {
        k_oops();
	}

    err = fogger_init(&fogger_callback);
    if (err) {
        k_oops();
    }

    return 0;
}
