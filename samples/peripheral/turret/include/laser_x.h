/*
 * Copyright (c) 2020 Daniel Veilleux
 */

#ifndef LASER_X_H__
#define LASER_X_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Empirical measurements:
 *
 * Preamble:     ~6ms
 * Short high    [0.40, 0.48]ms
 * Short low:    [0.52, 0.57]ms
 * Long low:     [1.47, 1.56]ms
 * ------------------------------
 * Total length: [16.7, 17.6]ms
 *
 * NOTE: Assumes an 8MHz clock so the 37.9KHz period is ~211 ticks.
 */

/* NOTE: Blasters regenerate 1 hitpoint per minute if they are not shot. */
#define LASER_X_HP		8

#define LASER_X_DAMAGE	1

enum laser_x_team {
	LASER_X_TEAM_RED,
	LASER_X_TEAM_BLUE,
	LASER_X_TEAM_NEUTRAL, /* Can be shot by any team, including other NEUTRALs	 */
	LASER_X_TEAM_COUNT
};

/**
 * @brief Retrieve the refresh count and data buffer for the given cmd.
 *
 * @retval  0 success
 * @retval -1 invalid cmd
 * @retval 
 */
int laser_x_cmd_get(enum laser_x_team team,
	                uint32_t *refresh_count, const uint16_t **buf, uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif /* LASER_X_H__ */