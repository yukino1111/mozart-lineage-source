# Source provenance

This repository distinguishes imported work from locally maintained changes.
Commit authorship is preserved when a patch is copied or backported; a local
author is used only for the repository's original integration and device work.

## Device source

`device/huawei/mozart` is derived from
[`kirin930-dev/android_device_huawei_mozart`](https://github.com/kirin930-dev/android_device_huawei_mozart)
through commit `4a057c26bbc1b85782d10fb99907ec14974433a1`. Its history was replayed
without the proprietary `hw_healthd`, `oeminfo_nvm_server`, and `teecd`
executables; the original author names and commit messages were retained before
the maintained local changes.

The vendor and kernel baselines are pinned in `local_manifests/mozart.xml` and
remain separate upstream repositories. Only local differences are stored here.

## Retained third-party patches

- Hardware bitmap workaround: Bilux (`i.bilux@gmail.com`), with the patch also
  crediting TRONX2100 and RAWMAIN.
- Broadcom Wi-Fi HAL initialization compatibility: schwienernitzel
  (`pfelix0803@gmail.com`).
- Audio `setMasterVolume` compatibility: schwienernitzel.
- Bluetooth HCI parser compatibility: DarkJoker360
  (`simoespo159@gmail.com`).
- Legacy mkbootimg tags-offset handling: schwienernitzel.
- SurfaceFlinger framebuffer wake behavior: Eduardo Alonso
  (`edu@error404software.com`), signed off by Thespartann and adapted locally.
- ARM64 `NT_ARM_SYSTEM_CALL`: AKASHI Takahiro
  (`takahiro.akashi@linaro.org`), Linux commit
  `766a85d7bc5d7f1ddd6de28bdb844eae45ec63b0`.

The original mail-style headers are kept in the patch files. Local adaptations
are identified separately instead of replacing the original author.

## Removed inherited patches

The inherited device-assert bypass is not carried: the device tree already
declares `TARGET_OTA_ASSERT_DEVICE`, so bypassing the assertion is unnecessary.
The vendor security-patch-level hardcode is cosmetic and is not carried. The
disabled boot logcat patch is also omitted because it is never applied.
