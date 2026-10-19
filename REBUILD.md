# TrollSpeed source reconstruction

This repository branch contains a new iOS application project reconstructed from the supplied ARM64 executable. It is under verification, and is not a verified one-to-one replacement.

## Build

The `Rebuild iOS application` GitHub Actions workflow uses a macOS runner and Apple SDK. It restores the original 699 image assets from the hash-checked `归档.zip`, fetches Dear ImGui at a pinned commit, runs the portable checks, builds the application, and ad hoc signs/packages it for TrollStore. The generated IPA remains unverified on a physical device.

The portable implementation can be checked with `make check` after obtaining the pinned ImGui checkout. Generate the Xcode project with `python3 Tools/generate_project.py`.

## Included

- Mach process/memory access, original embedded offset profile, player/minion/monster extraction, projection and smoothing.
- Coordinate VM object discovery, guarded instruction updates and complete four-byte read-back checks.
- Metal/ImGui ESP drawing, original portrait/skill assets, native settings editor and private HUD hosting startup.
- A separate self-owned authorization server and signed-lease client. The server address is configurable in the app. Private issuer keys and administrator credentials are not committed.

## Verification boundaries

Portable checks and the original ARM64 geometry comparison do not establish device compatibility or current-game offset compatibility. Original UI layout, touch forwarding, all rendering details, authorization renewal during a full match, and real-device behavior remain under review. See `docs/RECONSTRUCTION.md` and compiler logs; do not interpret an uploaded build artifact as runtime proof.

Use only on devices and software you are permitted to inspect. The new authorization service does not unlock the original application.
