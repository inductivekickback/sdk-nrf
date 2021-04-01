/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <rad.h>
	
LOG_MODULE_REGISTER(rad_message_type_rad, CONFIG_RAD_MESSAGE_TYPE_RAD_LOG_LEVEL);

void rad_message_type_rad_init(void)
{
	// TODO:
}
