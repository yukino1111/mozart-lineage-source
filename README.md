# mozart-lineage-source: LineageOS 16.0

Maintained device source and upstream patch stack for Huawei MediaPad M2 8.0
(`mozart`).

This branch contains the final buildable `device/huawei/mozart` source directly.
Kernel, vendor, AOSP and LineageOS changes remain reproducible patches based on
exactly pinned [kirin930-dev](https://github.com/kirin930-dev) and LineageOS
revisions.

## Repository layout

- `device/huawei/mozart/` is the maintained final device tree, not a diff;
- `patches/upstream/kirin930/` preserves the public-project compatibility
  patches inherited from the upstream device tree;
- the other paths below `patches/` contain this branch's maintained changes to
  upstream repositories;
- `patches/final/` is the complete patch state used by the verified B217 user
  build; smaller historical patches are retained for attribution;
- `local_manifests/mozart.xml` pins device, vendor and kernel baselines to exact
  commits.

## Checkout

Initialize a normal LineageOS 16.0 tree, copy `local_manifests/mozart.xml` into
`.repo/local_manifests/`, sync, and run:

```sh
scripts/apply-local-patches.sh /android/lineage16-mozart
```

The script first installs this repository's complete device source and then
applies the final patch stack idempotently. The unchanged Huawei `hw_healthd`
executable remains supplied by the pinned device baseline; the B217
`oeminfo_nvm_server` and `teecd` executables are installed by the proprietary
extraction step. None of them are duplicated here.

The browser and WebView APKs are also deliberately excluded. Follow
`device/huawei/mozart/prebuilt/cromite/README.md` to provide the pinned Cromite
v138 pair before building.

The B217 binary set is likewise not redistributed. Run
`scripts/extract-proprietary-blobs.sh` with an extracted B217 `/system` tree;
all 239 files are checked against `proprietary-files/mozart-b217.txt` before
they are installed into the ignored local blob cache.

## Experimental DSS overlay

`patches/hardware/interfaces/hwc2onfbadapter-hisi-dss-overlay-fallback.patch`
adds an opt-in DSS overlay path to AOSP `HWC2OnFbAdapter` for mozart video
playback testing. It does not enable Huawei's proprietary
`hwcomposer.hi3635.so`.

The overlay path is disabled by default. Enable the conservative YUV-only path
on a flashed build with:

```sh
adb shell setprop persist.debug.mozart.hwc_overlay 1
adb reboot
```

RGB/RGBA app layers are intentionally not handled by this path. A temporary
RGB experiment caused tearing during app scrolling, so the maintained patch stays YUV-only.

If the DSS ioctl fails, SurfaceFlinger falls back to the existing fbdev/client
composition path for that process. `dumpsys SurfaceFlinger` includes a
`mozart_dss_overlay` line for quick status checks.

## Acknowledgements

The complete source and patch attribution audit is recorded in
[`PROVENANCE.md`](PROVENANCE.md). In particular, the published device tree
retains the Git ancestry of
[kirin930-dev/android_device_huawei_mozart](https://github.com/kirin930-dev/android_device_huawei_mozart),
and retained third-party patches keep their original author metadata.

## License

Unless otherwise noted, this repository's scripts, documentation, and local text
patches are licensed under the Apache License 2.0. This license does not apply
to third-party proprietary binaries, which are not included here.

Kernel-related patches, if added later, should be marked separately and follow
the upstream kernel license, `GPL-2.0-only`.
