# Cromite prebuilts

The LOS 18.1 image uses the matching Android 11-compatible Cromite browser and
System WebView pair from the official v148.0.7778.168 release:

<https://github.com/uazo/cromite/releases/tag/v148.0.7778.168-cb3baf14f52eb4365d017f640f85310735c19b79>

Expected files and SHA-256 digests:

- `Cromite.apk`: `77af7db8f0a02e8d8cd2099d1f9b5c8266d6ae4cba06924bda5c73f980dc6894`
- `CromiteSystemWebView.apk`: `dd690edc7ba909bfc095e798457cb874ab2d6ff1f63b980ed67cae5d725a8d14`

The APKs stay presigned so future updates from the same upstream signing key
remain installable.

`libchrome.so` and `libchrome_crashpad_handler.so` are extracted unchanged
from `Cromite.apk` into `lib/arm64-v8a/` during local setup. The build installs
them beside the byte-for-byte original APK so the read-only system app can load
its compressed JNI libraries without modifying the upstream-signed APK.
