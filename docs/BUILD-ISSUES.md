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

## Packaging shell syntax

- Expected: ad hoc sign and package after the Apple SDK build succeeds.
- What happened: the shell reported `syntax error: unexpected end of file` after `BUILD SUCCEEDED`.
- Where: signing/packaging step, run 39227882696.
- Repro: run the previous one-line conditional containing `find -exec … \; fi`.
- Cause: the escaped `\;` terminates find's command, not the shell's conditional body.
- Correction: put the find command and closing `fi` on separate lines. Packaging recheck required.

## Matrix data containing non-finite values

- Expected: reject unusable game matrices before they reach rendering and diagnostic JSON.
- What happened: implementation review found that a successfully read matrix containing NaN could pass the old zero-matrix check; JSON serialization does not accept NaN.
- Where: Core/GameData.cpp, capture(), identified without claiming a device crash.
- Repro: put NaN in the pointer-graph fixture's matrix while keeping matrix[0] nonzero.
- Correction: reject non-finite elements and reset cached match data. The added fault check passes.

## Cache retained across a missing actor root

- Expected: after match data disappears, the next match's first position is unfiltered and no old skill/recall state is reused.
- What happened: the missing-actor-root branch returned an invalid frame without clearing the reader caches.
- Where: Core/GameData.cpp, found by pointer-graph lifecycle verification.
- Repro: capture two different positions, remove the actor-array pointer, restore it, then capture again.
- Correction: reset caches when the actor root is unavailable. The fixture verifies smoothing within a match and a fresh unfiltered sample after reset.
