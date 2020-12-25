/*
 * Copyright (c) 2020 Daniel Veilleux
 */
#include <zephyr.h>

#include <nrfx_pwm.h>

#include "laser_x.h"


/*
 * Dynasty laser tag blaster IR codes.
 */
#define TICKS_PER_PERIOD    211
#define DUTY_CYCLE_0        TICKS_PER_PERIOD
#define DUTY_CYCLE_50       (TICKS_PER_PERIOD / 2)
#define REFRESH_COUNT_400US 15

int laser_x_cmd_get(enum laser_x_team team,
                    uint32_t *refresh_count, const uint16_t **buf, uint32_t *len)
{
    return 0;
}
