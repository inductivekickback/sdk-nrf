/*
 * Copyright (c) 2020 Daniel Veilleux
 */

#ifndef DYNASTY_H__
#define DYNASTY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Empirical measurements:
 *
 * Preamble:   ~1.6ms
 * Short high: [0.36, 0.41]ms
 * Long high:  [0.72, 0.75]ms
 * Short low:  [0.40, 0.44]ms
 * Long low:   [0.74, 0.8]ms
 * ------------------------------
 * Total length is [20.0, 20.8]ms
 */

#define DYNASTY_HP							9

#define DYNASTY_PISTOL_SHOT_COUNT			12
#define DYNASTY_SHOTGUN_MACHINE_GUN_COUNT	6
#define DYNASTY_ROCKET_COUNT				1

#define DYNASTY_PISTOL_DAMAGE				1
#define DYNASTY_SHOTGUN_MACHINE_GUN_DAMAGE	2
#define DYNASTY_ROCKET_DAMAGE				3

enum dynasty_team {
	DYNASTY_TEAM_RED,
	DYNASTY_TEAM_GREEN,
	DYNASTY_TEAM_BLUE,
	DYNASTY_TEAM_WHITE,
	DYNASTY_TEAM_COUNT
};

enum dynasty_weapon {
	DYNASTY_WEAPON_PISTOL,
	DYNASTY_WEAPON_SHOTGUN_MACHINE_GUN,
	DYNASTY_WEAPON_ROCKET,
	DYNASTY_WEAPON_COUNT
};

/**
 * @brief Retrieve the refresh count and data buffer for the given cmd.
 *
 * @retval  0 success
 * @retval -1 invalid cmd
 * @retval 
 */
int dynasty_cmd_get(enum dynasty_team team, enum dynasty_weapon weapon,
	                uint32_t *refresh_count, const uint16_t **buf, uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif /* DYNASTY_H__ */