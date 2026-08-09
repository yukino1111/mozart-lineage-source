/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "mozart_macaddr"

#include <log/log.h>

#include <array>
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr char kVendorHelper[] = "/vendor/bin/mac_addr_normalization_vendor";
constexpr char kBluetoothPath[] = "/data/misc/bluedroid/macbt";
constexpr char kWifiPath[] = "/data/misc/wifi/macwifi";

constexpr uid_t kSystemUid = 1000;
constexpr uid_t kBluetoothUid = 1002;
constexpr gid_t kBluetoothGid = 1002;
constexpr gid_t kWifiGid = 1010;

bool ReadFully(int fd, void* data, size_t size) {
    auto* current = static_cast<unsigned char*>(data);
    while (size > 0) {
        const ssize_t count = read(fd, current, size);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (count == 0) {
            return false;
        }
        current += count;
        size -= static_cast<size_t>(count);
    }
    return true;
}

bool WriteFully(int fd, const void* data, size_t size) {
    const auto* current = static_cast<const unsigned char*>(data);
    while (size > 0) {
        const ssize_t count = write(fd, current, size);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        current += count;
        size -= static_cast<size_t>(count);
    }
    return true;
}

int HexValue(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    return -1;
}

bool ParseMac(const std::string& text, std::array<unsigned char, 6>* address) {
    if (text.size() != 17) {
        return false;
    }

    for (size_t index = 0; index < address->size(); ++index) {
        const size_t offset = index * 3;
        const int high = HexValue(text[offset]);
        const int low = HexValue(text[offset + 1]);
        if (high < 0 || low < 0 || (index < address->size() - 1 && text[offset + 2] != ':')) {
            return false;
        }
        (*address)[index] = static_cast<unsigned char>((high << 4) | low);
    }

    bool all_zero = true;
    bool all_ff = true;
    for (const unsigned char value : *address) {
        all_zero &= value == 0;
        all_ff &= value == 0xff;
    }

    return !all_zero && !all_ff && (((*address)[0] & 0x01) == 0);
}

bool HasValidMac(const char* path) {
    const int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        return false;
    }

    char buffer[32] = {};
    ssize_t count;
    do {
        count = read(fd, buffer, sizeof(buffer));
    } while (count < 0 && errno == EINTR);
    close(fd);

    if (count <= 0) {
        return false;
    }

    std::string text(buffer, static_cast<size_t>(count));
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }

    std::array<unsigned char, 6> address = {};
    return ParseMac(text, &address);
}

void RunVendorHelper() {
    const pid_t child = fork();
    if (child < 0) {
        ALOGW("Unable to start vendor MAC helper: %s", strerror(errno));
        return;
    }

    if (child == 0) {
        execl(kVendorHelper, kVendorHelper, nullptr);
        _exit(127);
    }

    int status;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) {
            ALOGW("Unable to wait for vendor MAC helper: %s", strerror(errno));
            return;
        }
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        ALOGW("Vendor MAC helper did not complete successfully");
    }
}

bool GenerateMac(std::array<unsigned char, 6>* address) {
    const int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("Unable to open /dev/urandom: %s", strerror(errno));
        return false;
    }

    const bool success = ReadFully(fd, address->data(), address->size());
    close(fd);
    if (!success) {
        ALOGE("Unable to read /dev/urandom: %s", strerror(errno));
        return false;
    }

    (*address)[0] = static_cast<unsigned char>(((*address)[0] & 0xfe) | 0x02);
    return true;
}

bool StoreMac(const char* path, uid_t uid, gid_t gid) {
    std::array<unsigned char, 6> address = {};
    if (!GenerateMac(&address)) {
        return false;
    }

    char text[18];
    const int length = snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X",
                                static_cast<unsigned int>(address[0]),
                                static_cast<unsigned int>(address[1]),
                                static_cast<unsigned int>(address[2]),
                                static_cast<unsigned int>(address[3]),
                                static_cast<unsigned int>(address[4]),
                                static_cast<unsigned int>(address[5]));
    if (length != 17) {
        ALOGE("Unable to format fallback MAC address");
        return false;
    }

    const std::string temporary = std::string(path) + ".tmp";
    if (unlink(temporary.c_str()) < 0 && errno != ENOENT) {
        ALOGE("Unable to remove stale %s: %s", temporary.c_str(), strerror(errno));
        return false;
    }

    const int fd = open(temporary.c_str(),
                        O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0660);
    if (fd < 0) {
        ALOGE("Unable to create %s: %s", temporary.c_str(), strerror(errno));
        return false;
    }

    bool success = fchmod(fd, 0660) == 0 &&
                   fchown(fd, uid, gid) == 0 &&
                   WriteFully(fd, text, static_cast<size_t>(length)) &&
                   fsync(fd) == 0;
    if (close(fd) < 0) {
        success = false;
    }

    if (!success) {
        ALOGE("Unable to write %s: %s", temporary.c_str(), strerror(errno));
        unlink(temporary.c_str());
        return false;
    }

    if (rename(temporary.c_str(), path) < 0) {
        ALOGE("Unable to install %s: %s", path, strerror(errno));
        unlink(temporary.c_str());
        return false;
    }

    ALOGI("Generated and persisted a fallback address at %s", path);
    return true;
}

bool EnsureMac(const char* path, uid_t uid, gid_t gid) {
    return HasValidMac(path) || StoreMac(path, uid, gid);
}

}  // namespace

int main() {
    if (!HasValidMac(kBluetoothPath) || !HasValidMac(kWifiPath)) {
        RunVendorHelper();
    }

    const bool bluetooth_ok = EnsureMac(kBluetoothPath, kBluetoothUid, kBluetoothGid);
    const bool wifi_ok = EnsureMac(kWifiPath, kSystemUid, kWifiGid);
    return bluetooth_ok && wifi_ok ? 0 : 1;
}
