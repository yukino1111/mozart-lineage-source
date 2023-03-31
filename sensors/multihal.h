/*
 * Copyright (C) 2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HARDWARE_LIBHARDWARE_MODULES_SENSORS_MULTIHAL_H_
#define HARDWARE_LIBHARDWARE_MODULES_SENSORS_MULTIHAL_H_

#include <hardware/sensors.h>
#include <hardware/hardware.h>

static const char* MULTI_HAL_CONFIG_FILE_PATH = "/vendor/etc/sensors/_hals.conf";

// Depracated because system partition HAL config file does not satisfy treble requirements.
static const char* DEPRECATED_MULTI_HAL_CONFIG_FILE_PATH = "/system/etc/sensors/_hals.conf";

struct sensors_module_t *get_multi_hal_module_info(void);

#endif // HARDWARE_LIBHARDWARE_MODULES_SENSORS_MULTIHAL_H_
