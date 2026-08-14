/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Android.mk compiles compiler-rt's conversion routines directly so their
 * __aeabi_d2lz and __aeabi_d2ulz aliases stay visible to B217 camera blobs.
 */
int mozart_camera_legacy_shim;
