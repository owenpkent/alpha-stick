#pragma once

#include <cstdint>

#include "as_pipeline/profile.h"

namespace as {

// Loads the active profile from NVS (or defaults), exposes it, persists edits.
// JSON import/export over USB CDC lives in as_config/config_protocol.h.
void config_init();
const Profile &config_profile();
void config_set_profile(const Profile &p);  // applies and persists

// Bumped on every config_set_profile. Consumers that cached the profile (the
// sensing pipeline) poll this and re-read when it changes, so edits apply live
// without a reboot.
uint32_t config_generation();

}  // namespace as
