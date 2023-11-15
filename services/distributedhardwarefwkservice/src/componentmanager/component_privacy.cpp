/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "component_privacy.h"

#include "ability_manager_client.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "device_manager.h"
#include "dm_device_info.h"
#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentPrivacy"

ComponentPrivacy::ComponentPrivacy()
{
    DHLOGI("ComponentPrivacy dtor.");
}

ComponentPrivacy::~ComponentPrivacy()
{
    DHLOGI("ComponentPrivacy dtor.");
}

int32_t ComponentPrivacy::OnPrivaceResourceMessage(const ResourceEventType &type, const std::string &subType,
    const std::string &networkId, bool &isSensitive, bool &isSameAccout)
{
    DHLOGI("OnPrivaceResourceMessage start.");
    int32_t ret = DH_FWK_SUCCESS;
    if (type == ResourceEventType::EVENT_TYPE_QUERY_RESOURCE) {
        ret = OnResourceInfoCallback(subType, networkId, isSensitive, isSameAccout);
    }
    if (type == ResourceEventType::EVENT_TYPE_PULL_UP_PAGE) {
        ret = StartPrivacePage(subType, networkId);
    }
    return ret;
}

int32_t ComponentPrivacy::OnResourceInfoCallback(const std::string &subType, const std::string &networkId,
    bool &isSensitive, bool &isSameAccout)
{
    DHLOGI("OnResourceInfoCallback start.");
    isSensitive = isSensitive_;
    isSameAccout = isSameAccout_;
    return DH_FWK_SUCCESS;
}

int32_t ComponentPrivacy::StartPrivacePage(const std::string &subType, const std::string &networkId)
{
    DHLOGI("StartPrivacePage start.");
    DmDeviceInfo deviceInfo;
    DeviceManager::GetInstance().GetDeviceInfo(DH_FWK_PKG_NAME, networkId, deviceInfo);
    std::string deviceName = std::string(deviceInfo.deviceName);
    uint16_t deviceTypeId = deviceInfo.deviceTypeId;
    std::string deviceType = DeviceTypeToString(deviceTypeId);
    const std::string bundleName = "com.ohos.privacyview";
    const std::string abilityName = "PrivacyViewAbility";
    int32_t type = -1;
    if (subType == "AUDIO_MIC") {
        type = static_cast<int32_t>(DHType::AUDIO_MIC);
    } else if (subType == "CAMERA") {
        type = static_cast<int32_t>(DHType::CAMERA);
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    want.SetParam("type", type);
    want.SetParam("srcNetworkId", networkId);
    want.SetParam("deviceName", deviceName);
    want.SetParam("deviceType", deviceType);
    auto abilityManager = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        DHLOGE("AbilityManagerClient is nullptr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t result = abilityManager->StartAbility(want);
    DHLOGI("performance time: StartPrivacePage result = %d", result);
    return result;
}

void ComponentPrivacy::SetSensitiveResource(bool &isSensitive, bool &isSameAccout)
{
    isSensitive_ = isSensitive;
    isSameAccout_ = isSameAccout;
}

std::string ComponentPrivacy::DeviceTypeToString(uint16_t deviceTypeId)
{
    DHLOGI("DeviceTypeToString start.");
    DmDeviceType deviceType = static_cast<DmDeviceType>(deviceTypeId);
    switch (deviceType) {
        case DmDeviceType::DEVICE_TYPE_WIFI_CAMERA:
            return "camera";
        case DmDeviceType::DEVICE_TYPE_AUDIO:
            return "audio";
        case DmDeviceType::DEVICE_TYPE_PC:
            return "pc";
        case DmDeviceType::DEVICE_TYPE_PHONE:
            return "phone";
        case DmDeviceType::DEVICE_TYPE_PAD:
            return "pad";
        case DmDeviceType::DEVICE_TYPE_WATCH:
            return "watch";
        case DmDeviceType::DEVICE_TYPE_CAR:
            return "car";
        case DmDeviceType::DEVICE_TYPE_TV:
            return "tv";
        case DmDeviceType::DEVICE_TYPE_SMART_DISPLAY:
            return "display";
        case DmDeviceType::DEVICE_TYPE_2IN1:
            return "2in1";
        default:
            return "unknown";
    }
}
} // namespace DistributedHardware
} // namespace OHOS
