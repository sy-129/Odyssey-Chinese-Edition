# Reconstruction status

Target supplied by user: iPhone 13 Pro, iOS 16.5, TrollStore; game stated as Chinese release 12.1.1.1.

## Observations

The original ARM64 executable is unencrypted. Its embedded profile contains 88 offsets, including selected VM opcode locations. Coordinate initialization checks the tersafe module signature, scans candidate VM objects, changes selected STORE instructions to NOP, and checks memory again. It is not evidence of a general cryptographic cipher.

The original has Mach task access, a persona-spawned HUD, private system window hosting, and Metal/ImGui rendering. The source branch implements these paths with explicit failure reporting, rather than treating a successful spawn as a successful overlay.

## Checked locally

- 25 memory/Mach-O parser and geometry checks.
- 12 coordinate discovery, retry, memory-fault, full nine-PC profile and fresh-HUD restart checks.
- 11 data pointer-graph checks, including original visible skill-slot numbering, non-finite matrix rejection and match-cache reset.
- Real Dear ImGui populated/empty-frame geometry emission checks.
- 800 numerical comparisons of the original isolated ARM64 projection/minimap routines with the rewritten C++ routines; exact finite-case agreement in recorded runs.
- Authorization server: 22 HTTP checks, including automatic-renewal token rotation, replay/device rejection and revocation.

Fixtures are isolated verification inputs, never an application data source or a representation of actual game capture.

## Open work and unknowns

- Apple SDK compilation/linking succeeded in run 39227882696; packaging and latest changes are being rechecked.
- Rendered images have not been compared on the target device; visual parity and original upstream ImGui WIP differences remain unresolved.
- Floating control, settings menu and HID/AX input are implemented; exact original menu layout and device input delivery still need verification. The native main-app settings editor is a replacement interface.
- Game profile compatibility with the specified game release is not established by static analysis.
- Device-only checks: entitlement effectiveness, task access, private HUD startup/window orientation, real coordinates and overlays.
- The authorization service is not deployed; the user requested a configurable address. Automatic signed-lease renewal is implemented and server-tested; background behavior on the target device needs verification.
- The client is not claimed to be unbreakable against a person controlling the device or modifying the executable.

No item above is marked complete based only on successful compilation or a generated IPA.

## HUD interaction reconstruction

A floating ImGui control, three settings groups, dragging and hit-region pass-through are now implemented. The input path resolves `BKSHIDEventRegisterEventCallback`, wraps its fourth callback argument with `AXEventRepresentation`, then reads `location`, `handInfo.paths.pathIdentity` and touch-phase selectors, as recovered from original 0x10000b294 / 0x10000b570. It feeds the owned ImGui interface directly; it does not synthesize UITouch objects or alter the game's input.

Five headless interaction/settings-validation checks pass. This establishes menu state logic, not actual iOS HID delivery or exact original menu appearance. The original floating-control JPEG and app icon are now restored from the hash-checked archive; exact dimensions and visual parity still need device comparison.

All 58 settings getters were recovered after correcting decimal-offset parsing for heroBoxColorR at original field +100. UI and storage include that field. Application/keychain identifiers were explicitly added for the new client's own Keychain item; TrollStore's official entitlement example uses an unprefixed application-identifier. Keychain behavior still needs real-device verification.

## Rotation and screen lock

Original 0x10000992c / 0x10000ad58 observe FBS orientation changes and rotate the HUD view by +pi/2 for orientation 3 and -pi/2 for 4, coercing portrait updates to 3. A portrait-only UIKit window with a manually rotated landscape drawing view is now implemented to follow that behavior. The direct AX touch coordinates are converted through the transformed view. The original lock-state notification and screen-lock query are also restored. These private APIs require device validation; successful SDK linking does not exercise them.

## Skill texture fallback

The original instruction at 0x100032664 loads integer 0xd903 (55555) for the final bundled skill fallback, and the constant integer object at 0x100332fd8 contains the same value. The supplied archive includes wztx/55555.png. Missing primary/alternate skills now use this asset before falling back to the generated red disc. The original remappings 80116→80104 and 80117→80118 remain intact.

## Device diagnostic evidence

The main app's copy-diagnostics action includes startup messages plus a timestamped report from the real reader thread: target PID/module, coordinate read-back status/count, matrix, entity counts, and up to five actual observed hero samples. It contains no administrator password, issuer key, license code or refresh token. Reports are rate-limited to once per second. No data fixture is connected to the app. A non-finite matrix is rejected before rendering or JSON serialization; the data pointer-graph checks include that case.

HUD numeric controls use bounded touch-drag widgets rather than relying on an unavailable ImGui on-screen keyboard. Main-app numeric fields continue to use UIKit's keyboard. Menu text/background colors and navigation/content font scales are wired to the recovered settings. These interaction improvements are explicit reconstruction choices, not evidence of pixel-for-pixel menu parity.

## Direct comparison with original coordinate machine code

`Tools/compare_coordinate_vm.py` executes original routine 0x100018b60 in Unicorn and compares its result with `Core/CoordinateAccess.cpp` through a native bridge. Twenty-two scenarios pass with identical patch count, matched-PC mask and all four MiB of resulting remote-memory fixture bytes. Scenarios include 0–9 matching STORE locations, invalid object/state/opcodes/mode/length, denied/corrupted writes, partial read-back and an already-NOP candidate.

Task lookup, kernel memory I/O and vector allocation/deallocation are controlled boundaries. The original validator and patching instructions execute unchanged. This checks the candidate routine, not original region discovery, real game layout or iOS task permissions. Both this comparison and the 800-case geometry comparison now run in GitHub Actions and preserve JSON evidence.

Further drawing comparison corrected side-skill outlines/overlay opacity, the world recall badge's local rectangle and font scaling, the no-hero top-panel layout, and the app-level ESP master switch/data-ready gate recovered from original 0x100014f74 and renderImGuiMenu.

A fresh HUD can now recognize a fully initialized candidate only when all configured locations within the validated bytecode length contain four NOP bytes and the original module/object/state checks pass. A partial preexisting match is rejected. The original candidate routine still returns zero new writes in the all-NOP case; the C++ higher-level readiness recognition is an intentional restart repair and is tested separately from the byte-for-byte write-count/mask comparison.

## Packaged integrity

`Tools/verify_ipa.py` checks the embedded source-commit stamp, arm64/iOS deployment metadata, main executable CodeDirectory page/special hashes, sealed resource hashes, signed entitlements and original image bytes. It also checks that private issuer/admin state and reference/test material are absent from the app. Nested code entries without raw resource hashes are counted separately rather than claimed independently verified. This tool makes no certificate-trust or device-runtime claim. Xcode PNG recompression/text stripping is disabled to retain the original icon bytes.
