/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The linker flag in Android.mk pulls __aeabi_d2ulz from the ARM compiler
 * runtime. B217's 32-bit libcamera_algo.so expects that legacy EABI export.
 */
int mozart_camera_legacy_shim;
