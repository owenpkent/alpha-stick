#pragma once

#include "cJSON.h"

#include "as_pipeline/profile.h"

namespace as {

// Serialize a Profile to the nested JSON shape documented in
// docs/CONFIGURATION.md (mount / deadzone / curve / filter / mouse). The
// caller owns the returned cJSON node.
cJSON *profile_to_cjson(const Profile &p);

// Patch a Profile in place from a JSON object. Only keys that are present and
// well-typed are applied; unknown keys (host-side buttons/macros that the
// firmware does not consume yet) are ignored. Pass a Profile preloaded with
// the current values so a partial object only changes what it names.
void profile_from_cjson(const cJSON *obj, Profile &p);

}  // namespace as
