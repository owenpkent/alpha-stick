#include "as_config/config_protocol.h"

#include <cstring>

#include "cJSON.h"

#include "as_config/config.h"
#include "as_config/config_json.h"

#ifndef AS_FW_VERSION
#define AS_FW_VERSION "2.0.0-dev"
#endif

namespace as {

namespace {

bool s_reboot_pending = false;

char *ok_only()
{
    cJSON *r = cJSON_CreateObject();
    cJSON_AddBoolToObject(r, "ok", true);
    char *s = cJSON_PrintUnformatted(r);
    cJSON_Delete(r);
    return s;
}

char *error(const char *msg)
{
    cJSON *r = cJSON_CreateObject();
    cJSON_AddBoolToObject(r, "ok", false);
    cJSON_AddStringToObject(r, "error", msg);
    char *s = cJSON_PrintUnformatted(r);
    cJSON_Delete(r);
    return s;
}

char *reply_with_profile(const Profile &p)
{
    cJSON *r = cJSON_CreateObject();
    cJSON_AddBoolToObject(r, "ok", true);
    cJSON_AddItemToObject(r, "profile", profile_to_cjson(p));
    char *s = cJSON_PrintUnformatted(r);
    cJSON_Delete(r);
    return s;
}

}  // namespace

char *config_protocol_handle_line(const char *line)
{
    if (!line) {
        return nullptr;
    }
    while (*line == ' ' || *line == '\t' || *line == '\r') {
        ++line;
    }
    if (*line == '\0') {
        return nullptr;
    }

    cJSON *req = cJSON_Parse(line);
    if (!req) {
        return error("bad json");
    }

    const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(req, "cmd");
    char *resp = nullptr;

    if (!cJSON_IsString(cmd) || !cmd->valuestring) {
        resp = error("missing cmd");
    } else if (strcmp(cmd->valuestring, "info") == 0) {
        cJSON *r = cJSON_CreateObject();
        cJSON_AddBoolToObject(r, "ok", true);
        cJSON_AddStringToObject(r, "device", "Alpha Stick V2");
        cJSON_AddStringToObject(r, "fw", AS_FW_VERSION);
        cJSON_AddStringToObject(r, "proto", "1");
        const Profile &p = config_profile();
        cJSON_AddStringToObject(r, "profile_name", p.name);
        resp = cJSON_PrintUnformatted(r);
        cJSON_Delete(r);
    } else if (strcmp(cmd->valuestring, "get") == 0) {
        resp = reply_with_profile(config_profile());
    } else if (strcmp(cmd->valuestring, "set") == 0) {
        const cJSON *pj = cJSON_GetObjectItemCaseSensitive(req, "profile");
        if (!cJSON_IsObject(pj)) {
            resp = error("missing profile object");
        } else {
            Profile p = config_profile();  // merge onto current
            profile_from_cjson(pj, p);
            config_set_profile(p);
            resp = ok_only();
        }
    } else if (strcmp(cmd->valuestring, "defaults") == 0) {
        Profile p{};  // struct in-class initializers are the firmware defaults
        config_set_profile(p);
        resp = reply_with_profile(p);
    } else if (strcmp(cmd->valuestring, "save") == 0) {
        resp = ok_only();  // set already persisted; explicit ack for the UI
    } else if (strcmp(cmd->valuestring, "reboot") == 0) {
        s_reboot_pending = true;
        resp = ok_only();
    } else {
        resp = error("unknown cmd");
    }

    cJSON_Delete(req);
    return resp;
}

bool config_protocol_take_reboot()
{
    bool v = s_reboot_pending;
    s_reboot_pending = false;
    return v;
}

}  // namespace as
