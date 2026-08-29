# Mozart LOS 18.1 HWC2 experiment

Status: **rejected for production; retained for research only**.

This directory archives the source-level HWC2 experiment that connected AOSP's
`HWC2OnFbAdapter` to the hi3635 DSS `HISIFB_OV_ONLINE_PLAY` ioctl. It is not
referenced by `scripts/apply-local-patches.sh`, `device.mk`, or the release build.

Contents:

- `0001-hwc2onfbadapter-add-mozart-dss-overlay.patch`: adapter-side DSS layer
  selection, buffer translation, scaling capability, and fence handling.
- `hwcomposer/`: the standalone `hwcomposer.hi3635` wrapper with kernel VSYNC
  support and a timer fallback.

## Result

The implementation became stable enough to boot Enforcing, survive display
power cycles, scroll Settings, and play a scaled YUV video without new DSS
underflow, ioctl, or fence errors. That stability did not translate into a
useful product result:

- RGB overlay made Cromite and the NFS pre-race scene less smooth than the
  framebuffer fallback.
- Restricting overlay to YUV restored Cromite, NFS, PiliPlus, and system
  animations to the fallback's behavior, but produced no visible improvement.
- NFS gameplay remained limited by rendering work before final composition.

The RGB regression occurred because offloading one bottom RGB layer did not
remove GPU composition of the remaining stack. SurfaceFlinger still generated
a full-screen client target, after which DSS read and blended both buffers.

## Known constraints

- Do not submit the full client target a second time as a DSS overlay; the stock
  gralloc framebuffer post already submits it and duplicate submission caused
  continuous `ldi underflow`.
- Scaled layers require `CAP_SCL | CAP_CROSS_SWITCH` on this driver.
- Pass a DEVICE layer's acquire fence to the DSS ioctl. Waiting synchronously in
  `setLayerBuffer` can deadlock against the previous DSS release fence.
- The Android 6 B217 HWC1 binary is not a substitute. Its zero-valued fences and
  primary-display timing are incompatible with Android 11's HWC2On1Adapter.
- A test integration also needs SurfaceFlinger read access to the Mozart fb0
  sysfs nodes. No SELinux rule is kept in the production device policy.

The detailed verified failures remain in the control repository's
`knowledge/los18/ERROR_LOG.md`. Any future revival should start as a new
experiment and prove an end-user or power/bandwidth benefit against the stock
framebuffer fallback before re-entering the release path.
