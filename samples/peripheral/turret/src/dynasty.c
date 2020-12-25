/*
 * Copyright (c) 2020 Daniel Veilleux
 */
#include <zephyr.h>

#include <nrfx_pwm.h>

#include "dynasty.h"

/*
 * Dynasty Toys laser tag blaster IR codes
 */
#define TICKS_PER_PERIOD    211
#define DUTY_CYCLE_0        TICKS_PER_PERIOD
#define DUTY_CYCLE_50       (TICKS_PER_PERIOD / 2)
#define REFRESH_COUNT_400US 15

#define PREAMBLE DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50,DUTY_CYCLE_50
#define SHORT_1  DUTY_CYCLE_50
#define LONG_1   DUTY_CYCLE_50,DUTY_CYCLE_50
#define SHORT_0  DUTY_CYCLE_0
#define LONG_0   DUTY_CYCLE_0,DUTY_CYCLE_0
#define SS       SHORT_0,SHORT_1
#define SL       SHORT_0,LONG_1
#define LS       LONG_0,SHORT_1
#define LL       LONG_0,LONG_1
#define PREFIX   PREAMBLE,SS,SS,SS,SS,LS,LS,LS,LS
#define END      DUTY_CYCLE_0

/*
 * NOTE: These values must be in RAM due to EasyDMA limitations.
 */
static uint16_t RED_PISTOL[]  = {PREFIX,SS,SS,SS,LS,SS,SS,SS,SL,SS,SS,SL,LL,END};
static uint16_t RED_SHOTGUN[] = {PREFIX,SS,SS,SS,LS,SS,SS,SS,LS,SS,SS,LS,SS,END};
static uint16_t RED_ROCKET[]  = {PREFIX,SS,SS,SS,LS,SS,SS,SS,LL,SS,SS,LS,SL,END};

static uint16_t GREEN_PISTOL[]  = {PREFIX,SS,SS,SS,LL,SS,SS,SS,SL,SS,SS,LS,SS,END};
static uint16_t GREEN_SHOTGUN[] = {PREFIX,SS,SS,SS,LL,SS,SS,SS,LS,SS,SS,LS,SL,END};
static uint16_t GREEN_ROCKET[]  = {PREFIX,SS,SS,SS,LL,SS,SS,SS,LL,SS,SS,LS,LL,END};

static uint16_t BLUE_PISTOL[]  = {PREFIX,SS,SS,SS,SL,SS,SS,SS,SL,SS,SS,SL,LS,END};
static uint16_t BLUE_SHOTGUN[] = {PREFIX,SS,SS,SS,SL,SS,SS,SS,LS,SS,SS,SL,LL,END};
static uint16_t BLUE_ROCKET[]  = {PREFIX,SS,SS,SS,SL,SS,SS,SS,LL,SS,SS,LS,SS,END};

static uint16_t WHITE_PISTOL[]  = {PREFIX,SS,SS,SL,SS,SS,SS,SS,SL,SS,SS,LS,SL,END};
static uint16_t WHITE_SHOTGUN[] = {PREFIX,SS,SS,SL,SS,SS,SS,SS,LS,SS,SS,LS,LL,END};
static uint16_t WHITE_ROCKET[]  = {PREFIX,SS,SS,SL,SS,SS,SS,SS,LL,SS,SS,LL,SS,END};

int dynasty_cmd_get(enum dynasty_team team, enum dynasty_weapon weapon,
                    uint32_t *refresh_count, const uint16_t **buf, uint32_t *len)
{
    switch (team) {
    case DYNASTY_TEAM_RED:
        switch (weapon) {
        case DYNASTY_WEAPON_PISTOL:
            *buf = &RED_PISTOL[0];
            *len = NRF_PWM_VALUES_LENGTH(RED_PISTOL);
            break;
        case DYNASTY_WEAPON_SHOTGUN_MACHINE_GUN:
            *buf = &RED_SHOTGUN[0];
            *len = NRF_PWM_VALUES_LENGTH(RED_SHOTGUN);
            break;
        case DYNASTY_WEAPON_ROCKET:
            *buf = &RED_ROCKET[0];
            *len = NRF_PWM_VALUES_LENGTH(RED_ROCKET);
            break;
        default:
            return -1;
        }
        break;
    case DYNASTY_TEAM_GREEN:
        switch (weapon) {
        case DYNASTY_WEAPON_PISTOL:
            *buf = &GREEN_PISTOL[0];
            *len = NRF_PWM_VALUES_LENGTH(GREEN_PISTOL);
            break;
        case DYNASTY_WEAPON_SHOTGUN_MACHINE_GUN:
            *buf = &GREEN_SHOTGUN[0];
            *len = NRF_PWM_VALUES_LENGTH(GREEN_SHOTGUN);
            break;
        case DYNASTY_WEAPON_ROCKET:
            *buf = &GREEN_ROCKET[0];
            *len = NRF_PWM_VALUES_LENGTH(GREEN_ROCKET);
            break;
        default:
            return -1;
        }
        break;
    case DYNASTY_TEAM_BLUE:
        switch (weapon) {
        case DYNASTY_WEAPON_PISTOL:
            *buf = &BLUE_PISTOL[0];
            *len = NRF_PWM_VALUES_LENGTH(BLUE_PISTOL);
            break;
        case DYNASTY_WEAPON_SHOTGUN_MACHINE_GUN:
            *buf = &BLUE_SHOTGUN[0];
            *len = NRF_PWM_VALUES_LENGTH(BLUE_SHOTGUN);
            break;
        case DYNASTY_WEAPON_ROCKET:
            *buf = &BLUE_ROCKET[0];
            *len = NRF_PWM_VALUES_LENGTH(BLUE_ROCKET);
            break;
        default:
            return -1;
        }
        break;
    case DYNASTY_TEAM_WHITE:
        switch (weapon) {
        case DYNASTY_WEAPON_PISTOL:
            *buf = &WHITE_PISTOL[0];
            *len = NRF_PWM_VALUES_LENGTH(WHITE_PISTOL);
            break;
        case DYNASTY_WEAPON_SHOTGUN_MACHINE_GUN:
            *buf = &WHITE_SHOTGUN[0];
            *len = NRF_PWM_VALUES_LENGTH(WHITE_SHOTGUN);
            break;
        case DYNASTY_WEAPON_ROCKET:
            *buf = &WHITE_ROCKET[0];
            *len = NRF_PWM_VALUES_LENGTH(WHITE_ROCKET);
            break;
        default:
            return -1;
        }
        break;
    default:
        return -1;
    }
    *refresh_count = REFRESH_COUNT_400US;
    return 0;
}
