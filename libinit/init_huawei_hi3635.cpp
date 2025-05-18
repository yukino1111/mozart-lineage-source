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

void fix_fingerprints(std::string model) {
    if (model.find("ALE") != std::string::npos) {
        set_ro_build_prop("fingerprint", "Huawei/ALE-L21/hwALE-H:6.0/HuaweiALE-L21/C432B596:user/release-keys", false);
        set_ro_build_prop("description", "ALE-L21-user 6.0 HuaweiALE-L21 C432B596 release-keys", false);
    } else if (model.find("CAM") != std::string::npos) {
        set_ro_build_prop("fingerprint", "HUAWEI/CAM-L21/HWCAM-H:6.0/HUAWEICAM-L21/C900B197:user/release-keys", false);
        set_ro_build_prop("description", "CAM-L21-user 6.0 HUAWEICAM-L21 C900B197 release-keys", false);
    }
}

void vendor_load_properties() {
    std::string model;

    if (android::base::ReadFileToString(PRODUCT_NAME, &model)) {
        LOG(INFO) << "Found product name: " << model;
        set_ro_build_prop("model", model);

        if (model.find("ALE") != std::string::npos) {
            LOG(INFO) << "Enabling NFC for " << model;
            property_override("ro.boot.product.hardware.sku", "nfc");
        }
    }

    fix_fingerprints(model);
}
