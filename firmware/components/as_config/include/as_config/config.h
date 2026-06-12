#pragma once

#include "as_pipeline/profile.h"

namespace as {

// Loads the active profile from NVS (or defaults), exposes it, persists edits.
// JSON import/export and the multi-profile store arrive with as_web.
void config_init();
const Profile &config_profile();
void config_set_profile(const Profile &p);  // applies and persists

}  // namespace as
