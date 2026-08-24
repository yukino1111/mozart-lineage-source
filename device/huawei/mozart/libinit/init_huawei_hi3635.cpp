/*
 * Copyright (C) 2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"
#include "property_service.h"

#define PRODUCT_NAME "sys/firmware/devicetree/base/hisi,boardname"

constexpr char kStockBuildDescription[] =
        "M2-user 6.0 HUAWEIM2-801W C233B217 release-keys";
constexpr char kStockBuildDisplayId[] = "M2-801WV100R001C233B217";
constexpr char kStockBuildFingerprint[] =
        "HUAWEI/M2/HWMozart:6.0/HUAWEIM2-801W/C233B217:user/release-keys";

using std::string;

std::vector<string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
};

void property_override(string prop, string value) {
    auto pi = (prop_info*) __system_property_find(prop.c_str());

    if (pi != nullptr)
        __system_property_update(pi, value.c_str(), value.size());
    else
        __system_property_add(prop.c_str(), prop.size(), value.c_str(), value.size());
}

void set_ro_build_prop(const string &prop, const string &value, bool product = true) {
    string prop_name;

    for (const auto &source : ro_props_default_source_order) {
        if (product)
            prop_name = "ro.product." + source + prop;
        else
            prop_name = "ro." + source + "build." + prop;

        property_override(prop_name.c_str(), value.c_str());
    }
}

void vendor_load_properties() {
    std::string model;

    set_ro_build_prop("brand", "HUAWEI");
    set_ro_build_prop("device", "HWMozart");
    set_ro_build_prop("manufacturer", "HUAWEI");
    set_ro_build_prop("name", "M2");

    if (android::base::ReadFileToString(PRODUCT_NAME, &model)) {
        if (model.find("801W") != std::string::npos) {
            set_ro_build_prop("model", "HUAWEI M2-801W");
        }
        else if (model.find("801L") != std::string::npos) {
            set_ro_build_prop("model", "HUAWEI M2-801L");
        }
        else if (model.find("802L") != std::string::npos) {
            set_ro_build_prop("model", "HUAWEI M2-802L");
        }
        else if (model.find("803L") != std::string::npos) {
            set_ro_build_prop("model", "HUAWEI M2-803L");
        }
        else {
            set_ro_build_prop("model", "HUAWEI MediaPad M2 8.0");
        }
    }

    set_ro_build_prop("description", kStockBuildDescription, false);
    set_ro_build_prop("fingerprint", kStockBuildFingerprint, false);
    set_ro_build_prop("id", "HUAWEIM2-801W", false);
    set_ro_build_prop("version.incremental", "C233B217", false);
    property_override("ro.build.display.id", kStockBuildDisplayId);
    property_override("ro.bootimage.build.description", kStockBuildDescription);
    property_override("ro.bootimage.build.fingerprint", kStockBuildFingerprint);
    property_override("ro.bootimage.build.id", "HUAWEIM2-801W");
    property_override("ro.bootimage.build.version.incremental", "C233B217");
}
