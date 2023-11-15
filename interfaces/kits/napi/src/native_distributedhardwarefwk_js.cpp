/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "native_distributedhardwarefwk_js.h"

#include "device_type.h"
#include "ipc_skeleton.h"
#include "js_native_api.h"
#include "tokenid_kit.h"
#include "nlohmann/json.hpp"
#include "distributed_hardware_log.h"
#include "distributed_hardware_fwk_kit.h"

using namespace OHOS::DistributedHardware;

namespace {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr))

const int32_t DH_NAPI_ARGS_ONE = 1;
const int32_t DH_NAPI_ARGS_TWO = 2;

enum DHBussinessErrorCode {
    // Permission verify failed.
    ERR_NO_PERMISSION = 201,
    //The caller is not a system application.
    ERR_NOT_SYSTEM_APP = 202,
    // Input parameter error.
    ERR_INVALID_PARAMS = 401,
    // The distributed hardware is not started.
    ERR_CODE_DH_NOT_START = 24200101,
    // The source device is not connected.
    ERR_CODE_DEVICE_NOT_CONNECT = 24200102,
};

const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
const std::string ERR_MESSAGE_NO_PERMISSION = "Permission verify failed.";
const std::string ERR_MESSAGE_NOT_SYSTEM_APP = "The caller is not a system application.";
const std::string ERR_MESSAGE_DH_NOT_START = "The distributed hardware is not started.";
const std::string ERR_MESSAGE_DEVICE_NOT_CONNECT = "The source device is not connected.";

bool CheckArgsType(napi_env env, bool assertion, const std::string &paramName, const std::string &type)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + "The type of " + paramName +
                " must be " + type;
        napi_throw_error(env, std::to_string(ERR_INVALID_PARAMS).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool IsFunctionType(napi_env env, napi_value value)
{
    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, value, &eventHandleType);
    return CheckArgsType(env, eventHandleType == napi_function, "callback", "function");
}

napi_value GenerateBusinessError(napi_env env, int32_t err, const std::string &msg)
{
    napi_value businessError = nullptr;
    NAPI_CALL(env, napi_create_object(env, &businessError));
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, err, &errorCode));
    napi_value errorMessage = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errorMessage));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", errorCode));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", errorMessage));

    return businessError;
}

napi_value CreateErrorForCall(napi_env env, int32_t code, const std::string &errMsg, bool isAsync = true)
{
    DHLOGI("CreateErrorForCall code:%d, message:%s", code, errMsg.c_str());
    napi_value error = nullptr;
    if (isAsync) {
        napi_throw_error(env, std::to_string(code).c_str(), errMsg.c_str());
    } else {
        error = GenerateBusinessError(env, code, errMsg);
    }
    return error;
}

napi_value CreateBusinessError(napi_env env, int32_t errCode, bool isAsync = true)
{
    napi_value error = nullptr;
    switch (errCode) {
        case ERR_DM_NO_PERMISSION:
            error = CreateErrorForCall(env, ERR_NO_PERMISSION, ERR_MESSAGE_NO_PERMISSION, isAsync);
            break;
        case ERR_DM_DISCOVERY_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_DISCOVERY_INVALID, ERR_MESSAGE_DISCOVERY_INVALID, isAsync);
            break;
        case ERR_DM_PUBLISH_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_PUBLISH_INVALID, ERR_MESSAGE_PUBLISH_INVALID, isAsync);
            break;
        case ERR_DM_AUTH_BUSINESS_BUSY:
            error = CreateErrorForCall(env, DM_ERR_AUTHENTICALTION_INVALID,
                ERR_MESSAGE_AUTHENTICALTION_INVALID, isAsync);
            break;
        case ERR_DM_INPUT_PARA_INVALID:
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
            error = CreateErrorForCall(env, ERR_INVALID_PARAMS, ERR_MESSAGE_INVALID_PARAMS, isAsync);
            break;
        case ERR_DM_INIT_FAILED:
            error = CreateErrorForCall(env, DM_ERR_OBTAIN_SERVICE, ERR_MESSAGE_OBTAIN_SERVICE, isAsync);
            break;
        case ERR_NOT_SYSTEM_APP:
            error = CreateErrorForCall(env, ERR_NOT_SYSTEM_APP, ERR_MESSAGE_NOT_SYSTEM_APP, isAsync);
            break;
        default:
            error = CreateErrorForCall(env, DM_ERR_FAILED, ERR_MESSAGE_FAILED, isAsync);
            break;
    }
    return error;
}
} // namespace

void DistributedHardwareManager::JsObjectToString(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, char *dest, const int32_t destLen)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        if (!CheckArgsType(env, valueType == napi_string, fieldStr.c_str(), "string")) {
            return;
        }
        size_t result = 0;
        NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, field, dest, destLen, &result));
    } else {
        DHLOGE("devicemanager napi js to str no property: %s", fieldStr.c_str());
    }
}

void DistributedHardwareManager::JsObjectToInt(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, int32_t &fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        if (!CheckArgsType(env, valueType == napi_number, fieldStr.c_str(), "number")) {
            return;
        }
        napi_get_value_int32(env, field, &fieldRef);
    } else {
        DHLOGE("devicemanager napi js to int no property: %s", fieldStr.c_str());
    }
}

bool DistributedHardwareManager::IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

napi_value DistributedHardwareManager::PauseDistributedHardware(napi_env env, napi_callback_info info)
{
    DHLOGI("PauseDistributedHardware in");
    if (!IsSystemApp()) {
        // CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    napi_value result = nullptr;
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, ((argc >= DH_NAPI_ARGS_ONE) && (argc <= DH_NAPI_ARGS_TWO)), "requires 1 or 2 parameter");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_object, "description", "object")) {
        return nullptr;
    }
    int32_t type = -1;
    char networkId[96];
    JsObjectToInt(env, argv[0], "type", type);
    DHType dhType = static_cast<DHType>(type);
    if (dhType == DHType::AUDIO_MIC || dhType == DHType::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    }
    JsObjectToString(env, argv[0], "srcNetworkId", networkId, sizeof(networkId));
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    if (argc == DH_NAPI_ARGS_ONE) {    // promise
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        int32_t ret = dhFwkKit->PauseDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("PauseDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
        return promise;
    } else if (argc == DH_NAPI_ARGS_TWO) {    // callback
        if (!IsFunctionType(env, argv[1])) {
            return nullptr;
        }
        int32_t ret = dhFwkKit->PauseDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("PauseDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DistributedHardwareManager::ResumeDistributedHardware(napi_env env, napi_callback_info info)
{
    DHLOGI("ResumeDistributedHardware in");
    if (!IsSystemApp()) {
        // CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    napi_value result = nullptr;
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, ((argc >= DH_NAPI_ARGS_ONE) && (argc <= DH_NAPI_ARGS_TWO)), "requires 1 or 2 parameter");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_object, "description", "object")) {
        return nullptr;
    }
    int32_t type = -1;
    char networkId[96];
    JsObjectToInt(env, argv[0], "type", type);
    DHType dhType = static_cast<DHType>(type);
    if (dhType == DHType::AUDIO_MIC || dhType == DHType::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    }
    JsObjectToString(env, argv[0], "srcNetworkId", networkId, sizeof(networkId));
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    if (argc == DH_NAPI_ARGS_ONE) {    // promise
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        int32_t ret = dhFwkKit->ResumeDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("ResumeDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
        return promise;
    } else if (argc == DH_NAPI_ARGS_TWO) {    // callback
        if (!IsFunctionType(env, argv[1])) {
            return nullptr;
        }
        int32_t ret = dhFwkKit->ResumeDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("ResumeDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DistributedHardwareManager::StopDistributedHardware(napi_env env, napi_callback_info info)
{
    DHLOGI("StopDistributedHardware in");
    if (!IsSystemApp()) {
        // CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    napi_value result = nullptr;
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, ((argc >= DH_NAPI_ARGS_ONE) && (argc <= DH_NAPI_ARGS_TWO)), "requires 1 or 2 parameter");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_object, "description", "object")) {
        return nullptr;
    }
    int32_t type = -1;
    char networkId[96];
    JsObjectToInt(env, argv[0], "type", type);
    DHType dhType = static_cast<DHType>(type);
    if (dhType == DHType::AUDIO_MIC || dhType == DHType::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    }
    JsObjectToString(env, argv[0], "srcNetworkId", networkId, sizeof(networkId));
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    if (argc == DH_NAPI_ARGS_ONE) {    // promise
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        int32_t ret = dhFwkKit->StopDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("StopDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
        return promise;
    } else if (argc == DH_NAPI_ARGS_TWO) {    // callback
        if (!IsFunctionType(env, argv[1])) {
            return nullptr;
        }
        int32_t ret = dhFwkKit->StopDistributedHardware(dhType, std::string(networkId));
        if (ret != 0) {
            DHLOGE("StopDistributedHardware for DHType: %d filed", (uint32_t)dhType);
        }
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DistributedHardwareManager::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor dhmProperties[] = {
        DECLARE_NAPI_FUNCTION("pauseDistributedHardware", PauseDistributedHardware),
        DECLARE_NAPI_FUNCTION("resumeDistributedHardware", ResumeDistributedHardware),
        DECLARE_NAPI_FUNCTION("stopDistributedHardware", StopDistributedHardware),
    };

    DHLOGI("DistributedHardwareManager::Init is called!");
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(dhmProperties) / sizeof(dhmProperties[0]),
        dhmProperties));
    DHLOGI("All functions are configured..");
    return exports;
}

/*
 * Function registering all props and functions of ohos.distributedhardware
 */
static napi_value Export(napi_env env, napi_value exports)
{
    DHLOGI("Export is called!");
    DistributedHardwareManager::Init(env, exports);
    return exports;
}

/*
 * module define
 */
static napi_module g_dhModule = {.nm_version = 1,
                                 .nm_flags = 0,
                                 .nm_filename = nullptr,
                                 .nm_register_func = Export,
                                 .nm_modname = "distributedHardware.hardwareManager",
                                 .nm_priv = ((void *)0),
                                 .reserved = {0}};

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    DHLOGI("RegisterModule is called!");
    napi_module_register(&g_dhModule);
}