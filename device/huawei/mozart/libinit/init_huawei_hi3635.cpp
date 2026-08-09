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
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"
#include "property_service.h"

#define PRODUCT_NAME "sys/firmware/devicetree/base/hisi,boardname"

#ifndef MOZART_BUILD_NUMBER
#define MOZART_BUILD_NUMBER "00000000"
#endif

#define MOZART_BUILD_FINGERPRINT \
    "huawei/mozart/hi3635:9/PQ3A.190801/" MOZART_BUILD_NUMBER ":user/release-keys"
#define MOZART_BUILD_DESCRIPTION \
    "lineage_mozart-user 9 PQ3A.190801.002 " MOZART_BUILD_NUMBER " release-keys"

using android::base::GetProperty;
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

    if (android::base::ReadFileToString(PRODUCT_NAME, &model)) {
        if (model.find("801W") != std::string::npos) {
            set_ro_build_prop("model", "M2-801W");
        }
        else if (model.find("801L") != std::string::npos) {
            set_ro_build_prop("model", "M2-801L");
        }
        else if (model.find("802L") != std::string::npos) {
            set_ro_build_prop("model", "M2-802L");
        }
        else if (model.find("803L") != std::string::npos) {
            set_ro_build_prop("model", "M2-803L");
        }
        else {
            set_ro_build_prop("model", "MediaPad M2 8.0");
        }
    }

    set_ro_build_prop("fingerprint", MOZART_BUILD_FINGERPRINT, false);
    set_ro_build_prop("description", MOZART_BUILD_DESCRIPTION, false);
}
