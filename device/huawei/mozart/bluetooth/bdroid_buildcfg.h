/*
 * Copyright (C) 2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int property_get(const char *key, char *value, const char *default_value);
#ifdef __cplusplus
}
#endif

inline const char* BtmGetDefaultName()
{
	char device[92];
	property_get("ro.product.model", device, "");

        if (strstr(device, "M2-801W") != NULL) {
            return "M2-801W";
        } else if (strstr(device, "M2-801L") != NULL) {
            return "M2-801L";
        } else if (strstr(device, "M2-802L") != NULL) {
            return "M2-802L";
        } else if (strstr(device, "M2-803L") != NULL) {
            return "M2-803L";
        }

	return "MediaPad M2 8.0";
}

#define BTM_DEF_LOCAL_NAME   BtmGetDefaultName()
#define BTM_BYPASS_EXTRA_ACL_SETUP TRUE

#endif
