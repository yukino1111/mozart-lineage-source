# Cromite prebuilts

The LOS 16 image uses the matching Android 9-compatible Cromite browser and
System WebView pair from the official v138.0.7204.169 release:

<https://github.com/uazo/cromite/releases/tag/v138.0.7204.169-bd85e1d8092b493c5aa87292c98065aba3087112>

Expected files and SHA-256 digests:

- `Cromite.apk`: `da2edfda13e83e2f89640d4b752468170ca5caf657db0f555071623f248e43f9`
- `CromiteSystemWebView.apk`: `e0d0baf7d3766fbe99ed8c0cb012ca77bb56fe1eb1fbdd21609c994d61c2bda8`

The APKs stay presigned so future updates from the same upstream signing key
remain installable.

`libchrome.so` and `libchrome_crashpad_handler.so` are extracted unchanged
from `Cromite.apk` into `lib/arm64-v8a/` during local setup. Android 9 does not
extract compressed JNI libraries for a read-only system app, so the build
installs them beside the byte-for-byte original APK.
