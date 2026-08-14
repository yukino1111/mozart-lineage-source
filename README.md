# mozart-lineage-source: LineageOS 18.1

Maintained device source and upstream patch stack for running LineageOS 18.1
on the Huawei MediaPad M2 8.0 (`mozart`, HiSilicon Kirin 930/hi3635).

This branch contains the final buildable `device/huawei/mozart` source directly.
It adapts pinned kirin930-dev vendor and kernel baselines to the LineageOS 18.1
platform with patches and reproducible preparation scripts.

## Repository layout

- `device/huawei/mozart/` is the maintained final device tree, not a diff;
- `patches/` contains changes to upstream AOSP, LineageOS, kernel and vendor
  repositories;
- `local_manifests/mozart.xml` pins the device, vendor and kernel baselines to
  exact commits;
- `scripts/install-device-tree.sh` installs the maintained device source over
  the pinned device baseline while preserving Huawei rootfs prebuilts and
  locally supplied Cromite APKs that are intentionally not committed here.

## Checkout

Initialize a normal LineageOS 18.1 source tree, copy
`local_manifests/mozart.xml` into `.repo/local_manifests/`, and sync. The local
manifest uses exact kirin930-dev commits so the three external device-specific
repositories cannot silently change underneath this branch.

First extract the strictly hashed B217 proprietary inputs from an official
`/system` extraction, then apply this branch's patches:

```sh
scripts/extract-proprietary-blobs.sh /android/lineage18.1-mozart /path/to/stock-system
scripts/apply-local-patches.sh /android/lineage18.1-mozart
```

The script installs this repository's complete device tree and then applies
idempotent patches relative to the clean Git revisions selected by the
manifest. Do not run the pinned device baseline's old `patches/install.sh`;
its Android 9 patches only apply partially to the Android 11 platform.

The image also expects the matching Cromite browser and System WebView APKs
documented in
`device/huawei/mozart/prebuilt/cromite/README.md`. They remain presigned and
outside this repository; the local preparation entry verifies their digests
and extracts the browser JNI libraries without rewriting either APK.

## Acknowledgements

The complete source and patch attribution audit is recorded in
[`PROVENANCE.md`](PROVENANCE.md). In particular, the published device tree
retains the Git ancestry of
[kirin930-dev/android_device_huawei_mozart](https://github.com/kirin930-dev/android_device_huawei_mozart),
and backports keep their upstream author metadata.

## License

Unless otherwise noted, this repository's scripts, documentation and local text
patches are licensed under the Apache License 2.0. This license does not apply
to third-party proprietary binaries, which are not included here.

Kernel patches follow `GPL-2.0-only`.
