# Reconstruction status

Target supplied by user: iPhone 13 Pro, iOS 16.5, TrollStore; game stated as Chinese release 12.1.1.1.

## Observations

The original ARM64 executable is unencrypted. Its embedded profile contains 88 offsets, including selected VM opcode locations. Coordinate initialization checks the tersafe module signature, scans candidate VM objects, changes selected STORE instructions to NOP, and checks memory again. It is not evidence of a general cryptographic cipher.

The original has Mach task access, a persona-spawned HUD, private system window hosting, and Metal/ImGui rendering. The source branch implements these paths with explicit failure reporting, rather than treating a successful spawn as a successful overlay.

## Checked locally

- 25 memory/Mach-O parser and geometry checks.
- 6 coordinate discovery, retry and memory-fault checks.
- Real Dear ImGui populated/empty-frame geometry emission checks.
- 800 numerical comparisons of the original isolated ARM64 projection/minimap routines with the rewritten C++ routines; exact finite-case agreement in recorded runs.
- Authorization server: 15 HTTP checks recorded separately.

Fixtures are isolated verification inputs, never an application data source or a representation of actual game capture.

## Open work and unknowns

- Apple SDK compilation and resulting corrections.
- Complete rendering parity: minimap recall badge, top-panel recall row, texture fallbacks, layout and scale details must be compared further.
- Original floating controls/menu layout and modern HID-to-touch event forwarding are not reconstructed. The current native settings editor is a replacement interface.
- Game profile compatibility with the specified game release is not established by static analysis.
- Device-only checks: entitlement effectiveness, task access, private HUD startup/window orientation, real coordinates and overlays.
- The authorization service is not deployed; the user requested a configurable address. The current short lease expires after 10 minutes without a new activation; uninterrupted renewal needs completion.
- The client is not claimed to be unbreakable against a person controlling the device or modifying the executable.

No item above is marked complete based only on successful compilation or a generated IPA.
