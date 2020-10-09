/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <bluetooth/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELEMENT_COUNT 2

/**
 * @brief Initialize GPIO and workqueue items
 */
const struct bt_mesh_comp *model_handler_init(void);

/**
 * @brief Set the specified element's status
 *
 * @retval  0 Success
 * @retval -1 module has not been initialized
 * @retval -2 element_index is invalid
 */
int model_handler_elem_update(uint8_t element_indx, bool status);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
