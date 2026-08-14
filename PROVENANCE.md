# Source provenance

This repository distinguishes imported work from locally maintained changes.
Commit authorship is preserved when code is copied or backported; a local
author is used only for original integration and device work.

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

- Composer 2.1 passthrough implementation: Chia-I Wu (`olv@google.com`), based
  on AOSP commit `be99ad6e757c3bb65d0b3febf68ec006b3897dbb`.
- Region `FatVector` revert: Tim Murray (`timmurray@google.com`), based on AOSP
  commit `b11abe70adc17204605946cbe4b71b6705fd19b1` and adapted to the final
  LineageOS 18.1 API.
- SurfaceFlinger framebuffer wake behavior: Eduardo Alonso
  (`edu@error404software.com`), signed off by Thespartann and adapted locally
  for the Android 11 power path.
- ARM64 `NT_ARM_SYSTEM_CALL`: AKASHI Takahiro
  (`takahiro.akashi@linaro.org`), Linux commit
  `766a85d7bc5d7f1ddd6de28bdb844eae45ec63b0`.
- Thread-friendly `/proc/<pid>/fd` permission check: Oleg Nesterov
  (`oleg@redhat.com`), Linux commit
  `54708d2858e79a2bdda10bf8a20c80eb96c20613`.
- Broadcom Wi-Fi initialization behavior was inspired by the Android 9 device
  patch from schwienernitzel (`pfelix0803@gmail.com`), but the Android 11
  validation and lifetime handling is a separate local implementation.

Each copied backport is stored as its own mail-style patch with the original
author. Mixed patches were split during the audit so unrelated local work is
not attributed to an upstream author and upstream work is not attributed to the
local maintainer.
