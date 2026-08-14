/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Huawei's Marshmallow camera configuration server only uses this helper for
// diagnostics. Android 11 moved it out of libutils, so keep the legacy ABI
// without retaining process-wide stack state in the proprietary executable.
extern "C" void mozart_process_call_stack_ctor(void*)
        __asm__("_ZN7android16ProcessCallStackC1Ev");
extern "C" void mozart_process_call_stack_ctor(void*) {}

extern "C" void mozart_process_call_stack_dtor(void*)
        __asm__("_ZN7android16ProcessCallStackD1Ev");
extern "C" void mozart_process_call_stack_dtor(void*) {}

extern "C" void mozart_process_call_stack_update(void*)
        __asm__("_ZN7android16ProcessCallStack6updateEv");
extern "C" void mozart_process_call_stack_update(void*) {}

extern "C" void mozart_process_call_stack_log(const void*, const char*, int, const char*)
        __asm__("_ZNK7android16ProcessCallStack3logEPKc19android_LogPriorityS2_");
extern "C" void mozart_process_call_stack_log(const void*, const char*, int, const char*) {}
