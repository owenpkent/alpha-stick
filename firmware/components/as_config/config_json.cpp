#include "as_config/config_json.h"

#include <cstring>

namespace as {

namespace {

// --- enum <-> string maps (human-readable JSON, matches CONFIGURATION.md) ---

const char *mode_to_str(Mode m)
{
    switch (m) {
        case Mode::GAMEPAD:  return "gamepad";
        case Mode::MOUSE:    return "mouse";
        case Mode::KEYBOARD: return "keyboard";
        case Mode::DUAL:     return "dual";
        case Mode::ATOS:     return "atos";
    }
    return "gamepad";
}

Mode mode_from_str(const char *s, Mode fallback)
{
    if (!s) return fallback;
    if (strcmp(s, "gamepad") == 0)  return Mode::GAMEPAD;
    if (strcmp(s, "mouse") == 0)    return Mode::MOUSE;
    if (strcmp(s, "keyboard") == 0) return Mode::KEYBOARD;
    if (strcmp(s, "dual") == 0)     return Mode::DUAL;
    if (strcmp(s, "atos") == 0)     return Mode::ATOS;
    return fallback;
}

const char *curve_to_str(CurveType c)
{
    switch (c) {
        case CurveType::LINEAR: return "linear";
        case CurveType::EXPO:   return "expo";
        case CurveType::LOG:    return "log";
        case CurveType::SCURVE: return "scurve";
    }
    return "expo";
}

CurveType curve_from_str(const char *s, CurveType fallback)
{
    if (!s) return fallback;
    if (strcmp(s, "linear") == 0) return CurveType::LINEAR;
    if (strcmp(s, "expo") == 0)   return CurveType::EXPO;
    if (strcmp(s, "log") == 0)    return CurveType::LOG;
    if (strcmp(s, "scurve") == 0) return CurveType::SCURVE;
    return fallback;
}

// --- typed readers that leave the target untouched on a missing/bad key ---

void read_float(const cJSON *parent, const char *key, float &out)
{
    const cJSON *n = cJSON_GetObjectItemCaseSensitive(parent, key);
    if (cJSON_IsNumber(n)) {
        out = (float)n->valuedouble;
    }
}

void read_bool(const cJSON *parent, const char *key, bool &out)
{
    const cJSON *n = cJSON_GetObjectItemCaseSensitive(parent, key);
    if (cJSON_IsBool(n)) {
        out = cJSON_IsTrue(n);
    }
}

}  // namespace

cJSON *profile_to_cjson(const Profile &p)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", p.name);
    cJSON_AddStringToObject(root, "mode", mode_to_str(p.mode));

    cJSON *mount = cJSON_AddObjectToObject(root, "mount");
    cJSON_AddNumberToObject(mount, "rotation_deg", p.rotation_deg);
    cJSON *tare = cJSON_AddArrayToObject(mount, "gravity_tare");
    cJSON_AddItemToArray(tare, cJSON_CreateNumber(p.gravity_tare_x));
    cJSON_AddItemToArray(tare, cJSON_CreateNumber(p.gravity_tare_y));
    cJSON_AddBoolToObject(mount, "invert_x", p.invert_x);
    cJSON_AddBoolToObject(mount, "invert_y", p.invert_y);

    cJSON *dz = cJSON_AddObjectToObject(root, "deadzone");
    cJSON_AddStringToObject(dz, "type", "radial");
    cJSON_AddNumberToObject(dz, "inner", p.deadzone_inner);
    cJSON_AddNumberToObject(dz, "outer", p.deadzone_outer);

    cJSON *cv = cJSON_AddObjectToObject(root, "curve");
    cJSON_AddStringToObject(cv, "type", curve_to_str(p.curve));
    cJSON_AddNumberToObject(cv, "gain", p.curve_gain);
    cJSON_AddNumberToObject(cv, "expo", p.curve_expo);

    cJSON *flt = cJSON_AddObjectToObject(root, "filter");
    cJSON *oe = cJSON_AddObjectToObject(flt, "one_euro");
    cJSON_AddNumberToObject(oe, "min_cutoff", p.oe_min_cutoff);
    cJSON_AddNumberToObject(oe, "beta", p.oe_beta);

    cJSON *mouse = cJSON_AddObjectToObject(root, "mouse");
    cJSON_AddNumberToObject(mouse, "max_px_s", p.mouse_max_px_s);

    return root;
}

void profile_from_cjson(const cJSON *obj, Profile &p)
{
    if (!cJSON_IsObject(obj)) {
        return;
    }

    const cJSON *name = cJSON_GetObjectItemCaseSensitive(obj, "name");
    if (cJSON_IsString(name) && name->valuestring) {
        strncpy(p.name, name->valuestring, sizeof(p.name) - 1);
        p.name[sizeof(p.name) - 1] = '\0';
    }

    const cJSON *mode = cJSON_GetObjectItemCaseSensitive(obj, "mode");
    if (cJSON_IsString(mode)) {
        p.mode = mode_from_str(mode->valuestring, p.mode);
    }

    const cJSON *mount = cJSON_GetObjectItemCaseSensitive(obj, "mount");
    if (cJSON_IsObject(mount)) {
        read_float(mount, "rotation_deg", p.rotation_deg);
        read_bool(mount, "invert_x", p.invert_x);
        read_bool(mount, "invert_y", p.invert_y);
        const cJSON *tare = cJSON_GetObjectItemCaseSensitive(mount, "gravity_tare");
        if (cJSON_IsArray(tare) && cJSON_GetArraySize(tare) == 2) {
            const cJSON *tx = cJSON_GetArrayItem(tare, 0);
            const cJSON *ty = cJSON_GetArrayItem(tare, 1);
            if (cJSON_IsNumber(tx)) p.gravity_tare_x = (float)tx->valuedouble;
            if (cJSON_IsNumber(ty)) p.gravity_tare_y = (float)ty->valuedouble;
        }
    }

    const cJSON *dz = cJSON_GetObjectItemCaseSensitive(obj, "deadzone");
    if (cJSON_IsObject(dz)) {
        read_float(dz, "inner", p.deadzone_inner);
        read_float(dz, "outer", p.deadzone_outer);
    }

    const cJSON *cv = cJSON_GetObjectItemCaseSensitive(obj, "curve");
    if (cJSON_IsObject(cv)) {
        const cJSON *t = cJSON_GetObjectItemCaseSensitive(cv, "type");
        if (cJSON_IsString(t)) p.curve = curve_from_str(t->valuestring, p.curve);
        read_float(cv, "gain", p.curve_gain);
        read_float(cv, "expo", p.curve_expo);
    }

    const cJSON *flt = cJSON_GetObjectItemCaseSensitive(obj, "filter");
    if (cJSON_IsObject(flt)) {
        const cJSON *oe = cJSON_GetObjectItemCaseSensitive(flt, "one_euro");
        if (cJSON_IsObject(oe)) {
            read_float(oe, "min_cutoff", p.oe_min_cutoff);
            read_float(oe, "beta", p.oe_beta);
        }
    }

    const cJSON *mouse = cJSON_GetObjectItemCaseSensitive(obj, "mouse");
    if (cJSON_IsObject(mouse)) {
        read_float(mouse, "max_px_s", p.mouse_max_px_s);
    }
}

}  // namespace as
