# Build and verification issues

## Apple SDK: unsupported Mach header

- Expected: compile memory reads/writes and region enumeration for arm64 iOS 16+.
- What happened: Xcode 16.4 / iPhoneOS 18.5 SDK rejected `mach/mach_vm.h` and the corresponding unavailable declarations.
- Where: `App/MachSession.mm`, GitHub Actions run 39227239023, compile step.
- Repro: build the first reconstruction commit with the `Rebuild iOS application` workflow.
- Cause: macOS Mach API header was used in an iOS target.
- Correction: use the iOS SDK's `vm_read_overwrite`, `vm_write`, and `vm_region_recurse` with `vm_address_t` and `vm_size_t`; these types hold full 64-bit addresses on the arm64 target. Rebuild required for confirmation.

## Apple SDK: Objective-C type name shadowing

- Expected: load real resource images into Metal textures.
- What happened: `id<MTLTexture>` was parsed incorrectly after the function parameter `int id` shadowed Objective-C's `id` type.
- Where: `App/MetalTextures.mm`, same workflow run.
- Repro: compile `MetalTextures.mm` from the first reconstruction commit with Apple Clang.
- Cause: identifier collision, rather than a missing Metal library.
- Correction: rename the numeric asset argument `assetID`; rebuild required for confirmation.

## Coordinate initialization validation

- Expected: repeated initialization recognizes previously verified bytes, and changing any of the four written bytes invalidates the cached result.
- What happened: a repeated scan found no STORE instructions after a prior patch; cached verification checked only one byte.
- Where: `Core/CoordinateAccess.cpp` during implementation review.
- Repro: initialize the coordinate memory fixture twice, then change the second byte at a patched location.
- Correction: retain candidate metadata, revalidate code address/length, and check all four bytes. Six fault/retry checks pass locally.

## Apple SDK: link symbols

- Expected: link the arm64 application after source compilation succeeds.
- What happened: the second completed build reported undefined `CGRectZero` and `vm_region_recurse` symbols.
- Where: link stage, run 39227618848.
- Repro: build commit 4e68f52d6cbb5d3782eff874bdac3d11ed2aa32c.
- Cause: CoreGraphics was not explicitly linked; the region API needed the `_64` symbol actually imported by the original application.
- Correction: add CoreGraphics and use `vm_region_recurse_64`, confirmed against the original REA/Ghidra imported-function inventory at 0x1000c9a6c. Rebuild required for confirmation.

## Visible skill indexing

- Expected: the first two displayed hero skills read profile slots skill1 and skill2, and readiness uses skill1/2/3/4.
- What happened: initial handwritten extraction used skill0/1 for cooldowns and skill0/1/2/3 for readiness.
- Where: Core/GameData.cpp, found by comparison with corrected GetPlayers pseudocode.
- Repro: the isolated pointer-graph fixture assigns distinct cooldowns and readiness to every raw skill slot.
- Correction: match DAT_10041db18/db1c/db20/db24 to the loader's skill1/2/3/4 mappings; seven data extraction checks pass.
