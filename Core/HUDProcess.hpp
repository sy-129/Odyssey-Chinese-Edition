#pragma once
#include <string>
#include <sys/types.h>
namespace ts {
struct LaunchResult { bool success; pid_t pid; int error; std::string message; };
// Recovered _SetHUDEnabled: spawn this executable in its -hud/-exit mode.
// The embedding application MUST implement those modes; this library does not create a HUD by itself.
LaunchResult spawnHUDMode(bool enable);
}
