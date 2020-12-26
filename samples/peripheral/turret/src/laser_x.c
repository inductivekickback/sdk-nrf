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
#define REFRESH_COUNT_5US   18

#define PREAMBLE DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50, \
                 DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50, \
                 DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50
#define SHORT_1  DUTY_CYCLE_50
#define LONG_1   DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50
#define SHORT_0  DUTY_CYCLE_0
#define SS       SHORT_0,SHORT_1
#define SL       SHORT_0,LONG_1
#define PREFIX   PREAMBLE,SS,SL,SS,SL,SS,SS
#define END      SHORT_0

/*
 * NOTE: These values must be in RAM due to EasyDMA limitations.
 */
static uint16_t BLUE[]    = {PREFIX,SS,SL,END};
static uint16_t RED[]     = {PREFIX,SL,SS,END};
static uint16_t NEUTRAL[] = {PREFIX,SL,SL,END};

int laser_x_cmd_get(enum laser_x_team team,
                    uint32_t *refresh_count, const uint16_t **buf, uint32_t *len)
{
    switch (team) {
    case LASER_X_TEAM_RED:
        *buf = &RED[0];
        *len = NRF_PWM_VALUES_LENGTH(RED);
        break;
    case LASER_X_TEAM_BLUE:
        *buf = &BLUE[0];
        *len = NRF_PWM_VALUES_LENGTH(BLUE);
        break;
    case LASER_X_TEAM_NEUTRAL:
        *buf = &NEUTRAL[0];
        *len = NRF_PWM_VALUES_LENGTH(NEUTRAL);
        break;
    default:
        return -1;
    }
    *refresh_count = REFRESH_COUNT_5US;
    return 0;
}
