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
the maintained local changes.

The vendor and kernel baselines are pinned in `local_manifests/mozart.xml` and
remain separate upstream repositories. Only local differences are stored here.

The complete tested integration is represented by the topic patches invoked by
`scripts/apply-local-patches.sh` from the exact revisions pinned by the local
manifest. Each patch is applied once: copied work names its original author,
mixed work names both the external source and local maintainer, and all
remaining patches state that they are locally maintained. The stack contains
only public source changes; B217 firmware binaries, Cromite APKs and release
signing material are excluded.
`proprietary-files/mozart-b217.txt` records hashes and extraction paths for the
stock files without redistributing them.

## Retained third-party patches

- `patches/upstream/kirin930/frameworks/base/Hardware-bitmaps-support-workaround.patch`:
  Bilux (`i.bilux@gmail.com`), with the patch also crediting TRONX2100 and
  RAWMAIN.
- `patches/upstream/kirin930/hardware/broadcom/wlan/WifiHAL-Do-not-error-check-on-initialization.patch`:
  schwienernitzel (`pfelix0803@gmail.com`).
- `patches/upstream/kirin930/hardware/interfaces/Audio-skip-setMasterVolume-if-not-implement.patch`:
  schwienernitzel.
- `patches/upstream/kirin930/system/bt/Hci-dont-crash-if-some-checks-fail.patch`:
  DarkJoker360 (`simoespo159@gmail.com`).
- `patches/upstream/kirin930/system/core/Support-mkbootimg-0xffb88000-as-tags-offset.patch`:
  schwienernitzel.
- `patches/frameworks/native/surfaceflinger-powerdown-lcd-on-off.patch`:
  framebuffer wake behavior by Eduardo Alonso (`edu@error404software.com`),
  signed off by Thespartann and adapted locally for LOS 16 power-down handling.
- `patches/kernel/huawei/mozart/arm64-nt-arm-system-call-regset.patch`:
  AKASHI Takahiro (`takahiro.akashi@linaro.org`), Linux commit
  `766a85d7bc5d7f1ddd6de28bdb844eae45ec63b0`.

The original mail-style headers are kept in the patch files. Local adaptations
are identified separately instead of replacing the original author.

The audit used byte-for-byte comparison against the pinned kirin930-dev device
patches, Git history and exact distinctive-code searches across the pinned
source trees and public GitHub results. No additional external patch match was
found. Unmatched work is therefore recorded as local rather than assigning an
unverified third-party author; new contrary evidence should replace this
classification.

## Derived interface definitions

`patches/hardware/interfaces/hwc2onfbadapter-hisi-dss-overlay-fallback.patch`
contains the minimum ioctl numbers, enum values and structure layouts required
to call the hi3630 DSS overlay ABI. Those declarations are derived from
`drivers/video/hisi/hi3630/hisi_dss.h` in the pinned Huawei kernel commit
`8f4fbfd523464cf1f36263c7e76a3e034d82d7b2`; the HWC2 adapter, validation,
fallback and diagnostics are local work. This origin is separate from AOSP's
pre-existing `HWC2OnFbAdapter` implementation.

## Cross-branch ports

The hi3635 power HAL, its narrow SELinux policy, the camera-recording software
encoder additions and Camera2 recording tap-to-focus additions were ported
from this maintainer's LineageOS 18.1 release stack at commit
`358d3e6949f3837ffbe0c8de2de9d32f9759de06`. They are not third-party upstream
imports. The LineageOS 16 versions were adapted to Android 9 APIs; the GPU boost
was retuned to the verified B217 600 MHz operating point.

The RDR post-fs-data correction is LineageOS 16-specific local work. The
LineageOS 18.1 kernel baseline still had the runnable `schedule_timeout(HZ)`
loop and `/dev/block` wait when this fix was made.

## Locally maintained patch inventory

- `build/make/mozart-release-ota-build-tools.patch`: deterministic build,
  target-files signing and legacy OTA tooling.
- `frameworks/av/img-msvdx-decoder-framerate-compat.patch`: IMG MSVDX OMX
  framerate compatibility.
- `frameworks/av/legacy-mozart-camera-recording.patch`: Android 9 port of the
  locally maintained LOS 18 camera-recording fix.
- `frameworks/base/packageinstaller-webview-compat.patch`,
  `gnss-geofence-native-timeout.patch` and `legacy-mozart-runtime-compat.patch`:
  package installation, WebView, media, lid and GNSS stability fixes.
- `frameworks/native/legacy-huawei-camera-abi.patch`: legacy Huawei gralloc and
  camera buffer-mapper ABI compatibility.
- `hardware/interfaces/legacy-private-sensor-type-compat.patch`,
  `hwc2onfbadapter-hisi-dss-overlay-fallback.patch` and
  `legacy-huawei-camera-hal.patch`: local sensor, optional DSS overlay and
  camera HAL compatibility; the DSS ioctl declarations have the Huawei kernel
  origin recorded above.
- `kernel/huawei/mozart/disable-debug-info.patch` and
  `legacy-mozart-b217-runtime.patch`: build-size choice, B217 Mali selection and
  the locally diagnosed RDR waiter correction.
- `packages/apps/Camera2/legacy-mozart-camera-behavior.patch`: Camera2
  compatibility, including the LOS 18 tap-focus port.
- `system/core/init-user-permissive-selinux.patch`: local userdebug diagnostic
  capability; release builds still boot Enforcing.
- `vendor/huawei/mozart/restore-emui31-gpu-omx-vendor-paths.patch`,
  `preserve-stock-mac-normalization-helper.patch` and
  `legacy-mozart-camera-vendor.patch`: local B217/legacy vendor layout.
- `vendor/lineage/disable-backuptool-and-hudson-fetch.patch`: local offline and
  release-build controls.

## Removed inherited patches

The inherited device-assert bypass is not carried: the device tree already
declares `TARGET_OTA_ASSERT_DEVICE`, so bypassing the assertion is unnecessary.
The vendor security-patch-level hardcode is cosmetic and is not carried. The
disabled boot logcat patch is also omitted because it is never applied.
