# Source provenance

This repository distinguishes imported work from locally maintained changes.
Every active patch starts with a short `# Provenance:` statement and points
back to this file. Imported mail headers are retained when available, but the
short statement and this ledger are the authoritative ownership summary.

## Device source

`device/huawei/mozart` is derived from
[`kirin930-dev/android_device_huawei_mozart`](https://github.com/kirin930-dev/android_device_huawei_mozart)
through commit `4a057c26bbc1b85782d10fb99907ec14974433a1`. Its history was replayed
without the proprietary `hw_healthd`, `oeminfo_nvm_server`, and `teecd`
executables; the original author names and commit messages were retained before
the maintained LineageOS 18.1 changes.

The final device uses the B217 `oeminfo_nvm_server` and `teecd` inputs selected
by the public SHA-1 manifest. These executables, the Huawei camera stack and
the other proprietary firmware inputs are never stored in this repository;
only their paths, digests and deterministic compatibility transformations are
published.

The vendor and kernel baselines are pinned in `local_manifests/mozart.xml` and
remain separate upstream repositories. Only their required differences are
stored here.

## Backports and adaptations

- `patches/hardware/interfaces/0001-composer-restore-the-2.1-passthrough-implementation.patch`:
  Composer 2.1 passthrough implementation by Chia-I Wu (`olv@google.com`), based
  on AOSP commit `be99ad6e757c3bb65d0b3febf68ec006b3897dbb`.
- `patches/frameworks/native/0001-frameworks-native-backport-Region-FatVector-revert.patch`:
  Region `FatVector` revert by Tim Murray (`timmurray@google.com`), based on AOSP
  commit `b11abe70adc17204605946cbe4b71b6705fd19b1` and adapted to the final
  LineageOS 18.1 API.
- `patches/frameworks/native/0002-surfaceflinger-control-the-legacy-framebuffer-power-.patch`:
  SurfaceFlinger framebuffer wake behavior by Eduardo Alonso
  (`edu@error404software.com`), signed off by Thespartann and adapted locally
  for the Android 11 power path.
- `patches/kernel/huawei/mozart/0002-arm64-ptrace-add-NT_ARM_SYSTEM_CALL-regset.patch`:
  ARM64 `NT_ARM_SYSTEM_CALL` by AKASHI Takahiro
  (`takahiro.akashi@linaro.org`), Linux commit
  `766a85d7bc5d7f1ddd6de28bdb844eae45ec63b0`.
- `patches/kernel/huawei/mozart/0004-proc-actually-make-proc_fd_permission-thread-friendl.patch`:
  thread-friendly `/proc/<pid>/fd` permission check by Oleg Nesterov
  (`oleg@redhat.com`), Linux commit
  `54708d2858e79a2bdda10bf8a20c80eb96c20613`.
- `patches/hardware/broadcom/wlan/legacy-bcmdhd-wifi-hal.patch`: Broadcom Wi-Fi
  initialization behavior was inspired by the Android 9 device
  patch from schwienernitzel (`pfelix0803@gmail.com`), but the Android 11
  validation and lifetime handling is a separate local implementation.

Each copied backport names its original author. Mixed patches name both the
external source and the local maintainer so unrelated local work is not
attributed to an upstream author and upstream work is not attributed to the
local maintainer.

## Locally maintained patch inventory

The available repository history records the following patches as Mozart
integration work maintained by yukino1111. They modify upstream Android code,
whose existing file copyrights and licenses remain in force. No separate
third-party patch origin has been identified; contrary source evidence must
replace this classification rather than being silently ignored:

- `patches/build/make/lineage18-mozart-build-ota.patch`
- `patches/frameworks/av/legacy-audio-version-table.patch`
- `patches/frameworks/av/legacy-mozart-camera-recording.patch`
- `patches/frameworks/base/legacy-cover-boot-broadcast.patch`
- `patches/frameworks/base/legacy-install-media-gnss-stability.patch`
- `patches/frameworks/base/legacy-mali-egl-main-thread.patch`
- `patches/frameworks/native/legacy-huawei-camera-abi.patch`
- `patches/hardware/interfaces/0002-wifi-validate-legacy-interface-handles.patch`
- `patches/hardware/interfaces/0003-light-synchronize-the-mozart-framebuffer-state.patch`
- `patches/hardware/interfaces/legacy-huawei-camera-hal1.patch`
- `patches/hardware/interfaces/legacy-private-sensor-types.patch`
- `patches/hardware/libhardware/legacy-mozart-gralloc-path.patch`
- `patches/hardware/lineage/interfaces/legacy-gnss-nmea-copy.patch`
- `patches/kernel/huawei/mozart/0001-mozart-disable-kernel-debug-information.patch`
- `patches/kernel/huawei/mozart/0003-ion-require-the-CMA-heap-device.patch`
- `patches/kernel/huawei/mozart/0005-mozart-use-b217-r8p0-mali.patch`
- `patches/kernel/huawei/mozart/legacy-fde-aes-compat.patch`
- `patches/packages/apps/Bluetooth/legacy-huawei-disable-scs.patch`
- `patches/packages/apps/Camera2/legacy-mozart-camera-behavior.patch`
- `patches/packages/modules/NetworkStack/legacy-kernel-tcp-info.patch`
- `patches/system/bt/legacy-huawei-disable-scs.patch`
- `patches/system/core/legacy-dm-uevent-compat.patch`
- `patches/system/core/legacy-first-stage-mount.patch`
- `patches/system/core/userdebug-adb-root-default.patch`
- `patches/system/sepolicy/legacy-fde-data-mirror-policy.patch`
- `patches/system/tools/mkbootimg/legacy-boot-addresses.patch`
- `patches/system/vold/legacy-fde-data-mirror-unmount.patch`
- `patches/vendor/huawei/mozart/lineage18-camera-vendor.patch`
- `patches/vendor/huawei/mozart/lineage18-vendor-layout.patch`
- `patches/vendor/lineage/disable-recovery-backuptool.patch`

The six patches described under “Backports and adaptations” are excluded from
this local-only list. In particular, the Broadcom Wi-Fi patch is a separate
Android 11 implementation but still credits schwienernitzel as its inspiration.
