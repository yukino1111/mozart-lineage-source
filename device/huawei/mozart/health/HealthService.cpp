/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>

#include <android-base/logging.h>
#include <batteryservice/BatteryService.h>
#include <health2/service.h>
#include <healthd/healthd.h>

namespace {

constexpr int kMilliToMicro = 1000;
constexpr char kChargeNowPath[] = "/sys/class/power_supply/Battery/charge_now";

void scaleToMicro(int* value) {
    if (*value > INT_MAX / kMilliToMicro || *value < INT_MIN / kMilliToMicro) {
        return;
    }
    *value *= kMilliToMicro;
}

void scaleToMicro(int64_t* value) {
    if (*value > INT64_MAX / kMilliToMicro || *value < INT64_MIN / kMilliToMicro) {
        return;
    }
    *value *= kMilliToMicro;
}

}  // namespace

void healthd_board_init(struct healthd_config* config) {
    // Huawei's legacy fuel-gauge driver exposes remaining charge as charge_now
    // and reports current and charge values in mA/mAh. Android Health expects
    // these values in uA/uAh.
    config->batteryChargeCounterPath = android::String8(kChargeNowPath);
    LOG(INFO) << "Mozart health config charge counter: "
              << config->batteryChargeCounterPath.string();
}

int healthd_board_battery_update(struct android::BatteryProperties* props) {
    scaleToMicro(&props->batteryCurrent);
    scaleToMicro(&props->batteryFullCharge);
    scaleToMicro(&props->batteryChargeCounter);
    static bool logged = false;
    if (!logged) {
        LOG(INFO) << "Mozart health units: current=" << props->batteryCurrent
                  << " full=" << props->batteryFullCharge
                  << " counter=" << props->batteryChargeCounter;
        logged = true;
    }
    return 0;
}

void healthd_board_battery_property_update(int id, int64_t* value) {
    switch (id) {
        case android::BATTERY_PROP_CHARGE_COUNTER:
        case android::BATTERY_PROP_CURRENT_NOW:
        case android::BATTERY_PROP_CURRENT_AVG:
            scaleToMicro(value);
            break;
        default:
            break;
    }
}

int main() {
    return health_service_main();
}
