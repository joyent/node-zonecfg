//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2019 Joyent, Inc.
//

//
// Node.js C binding for parts of illumos/SmartOS "libzonecfg"
// See "/usr/lib/libzonecfg.h" and, less so, "/usr/lib/zone.h".
//
// Dev Notes:
// - "zc_" prefix is used to namespace identifiers in this file.
//

#include <assert.h>
#include <libzonecfg.h>

#include <node_api.h>

#include "./n-api-helper-macros.h"

//
// This is a convenience wrapper around `napi_throw_error` that supports
// printf-style arguments for the error msg.
//
napi_status _zc_napi_throw_error(napi_env env, const char* code, char *msg_format, ...)
{
    va_list ap;
    char    msg[1024];  // 1024 bytes should be enough for anyone?

    assert(msg_format != NULL);

    va_start(ap, msg_format);
    (void) vsnprintf(msg, sizeof (msg), msg_format, ap);
    va_end(ap);

    return napi_throw_error(env, code, msg);
}


napi_value zc_zone_get_state(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    char zone_name[ZONENAME_MAX + 1];
    size_t num_bytes;
    zone_state_t state;
    int err;
    napi_value js_state;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], zone_name,
        ZONENAME_MAX+1, &num_bytes));

    if ((err = zone_get_state(zone_name, &state)) != Z_OK) {
        // TODO: Eventually would be nice to have the libzonecfg.h "Z_*"
        // names used as the error "code". E.g. `Z_NO_ZONE` for a bogus
        // zonename.
        _zc_napi_throw_error(env, NULL, "could not get zone \"%s\" state: %s",
            zone_name, zonecfg_strerror(err));
    }

    NAPI_CALL(env, napi_create_uint32(env, state, &js_state));
    return js_state;
}


napi_value zc_zone_state_str(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    zone_state_t state;
    char *state_str = NULL;
    napi_value js_state_str;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_uint32(env, argv[0], &state));

    state_str = zone_state_str(state);

    NAPI_CALL(env, napi_create_string_utf8(env, state_str, NAPI_AUTO_LENGTH,
        &js_state_str));
    return js_state_str;
}


napi_value zc_zone_get_state_str(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    char zone_name[ZONENAME_MAX + 1];
    size_t num_bytes;
    zone_state_t state;
    char *state_str = NULL;
    int err;
    napi_value js_state_str;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], zone_name,
        ZONENAME_MAX+1, &num_bytes));

    if ((err = zone_get_state(zone_name, &state)) != Z_OK) {
        // TODO: Eventually would be nice to have the libzonecfg.h "Z_*"
        // names used as the error "code".
        _zc_napi_throw_error(env, NULL, "could not get zone \"%s\" state: %s",
            zone_name, zonecfg_strerror(err));
    }

    state_str = zone_state_str(state);

    NAPI_CALL(env, napi_create_string_utf8(env, state_str, NAPI_AUTO_LENGTH,
        &js_state_str));
    return js_state_str;
}


static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor properties[] = {
        // Dev Notes:
        // - https://nodejs.org/api/n-api.html#n_api_napi_property_descriptor
        //   for fields.
        // - `0b010` is `napi_property_attributes` for enumerable, read-only.
        //   https://nodejs.org/api/n-api.html#n_api_napi_property_attributes

        // Raw API
        { "zone_get_state", NULL, zc_zone_get_state, NULL, NULL, NULL, 0b010, NULL },
        { "zone_state_str", NULL, zc_zone_state_str, NULL, NULL, NULL, 0b010, NULL },

        // Convenience API
        { "zone_get_state_str", NULL, zc_zone_get_state_str, NULL, NULL, NULL, 0b010, NULL },
    };

    NAPI_CALL(env, napi_define_properties(
          env, exports, sizeof(properties) / sizeof(*properties), properties));

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
