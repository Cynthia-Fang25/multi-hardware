/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "distributed_hardware_manager_factory.h"

#include <cstdlib>
#include <dlfcn.h>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>

#include "dm_device_info.h"

#include "anonymous_string.h"
#include "constants.h"
#include "device_manager.h"
#include "dh_context.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManagerFactory"

IMPLEMENT_SINGLE_INSTANCE(DistributedHardwareManagerFactory);
bool DistributedHardwareManagerFactory::InitLocalDevInfo()
{
    DHLOGI("InitLocalDevInfo start");
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() > 0 && deviceList.size() <= MAX_ONLINE_DEVICE_SIZE) {
        DHLOGI("There is other device online, on need just init db, use normal logic");
        return true;
    }
    auto initResult = DistributedHardwareManager::GetInstance().LocalInit();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("InitLocalDevInfo failed, errCode = %{public}d", initResult);
        return false;
    }
    DHLOGI("InitLocalDevInfo success, check is need exit");

    deviceList.clear();
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGI("After InitLocalDevInfo, no device online, exit dhfwk");
        ExitDHFWK();
    }
    return true;
}

bool DistributedHardwareManagerFactory::Init()
{
    DHLOGI("start");
    auto initResult = DistributedHardwareManager::GetInstance().Initialize();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("Initialize failed, errCode = %{public}d", initResult);
        return false;
    }
    isInit = true;
    DHLOGI("success");
    return true;
}

void DistributedHardwareManagerFactory::UnInit()
{
    DHLOGI("start");
    DHTraceStart(COMPONENT_UNLOAD_START);
    HiSysEventWriteMsg(DHFWK_EXIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa exit begin.");

    // release all the resources synchronously
    DistributedHardwareManager::GetInstance().Release();
    isInit = false;
    DHTraceEnd();
    CheckExitSAOrNot();
}

void DistributedHardwareManagerFactory::ExitDHFWK()
{
    DHLOGI("No device online or deviceList is over size, exit sa process");
    HiSysEventWriteMsg(DHFWK_EXIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa exit end.");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        DHLOGE("systemAbilityMgr is null");
        return;
    }
    int32_t ret = systemAbilityMgr->UnloadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("systemAbilityMgr UnLoadSystemAbility failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("systemAbilityMgr UnLoadSystemAbility success");
}

void DistributedHardwareManagerFactory::CheckExitSAOrNot()
{
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        ExitDHFWK();
        return;
    }

    DHLOGI("After uninit, DM report devices online, reinit");
    Init();
    for (const auto &deviceInfo : deviceList) {
        const auto networkId = std::string(deviceInfo.networkId);
        const auto uuid = GetUUIDBySoftBus(networkId);
        DHLOGI("Send trusted device online, networkId = %{public}s, uuid = %{public}s",
            GetAnonyString(networkId).c_str(),
            GetAnonyString(uuid).c_str());
        std::thread(&DistributedHardwareManagerFactory::SendOnLineEvent, this, networkId, uuid,
            deviceInfo.deviceTypeId).detach();
    }
}

bool DistributedHardwareManagerFactory::IsInit()
{
    return isInit.load();
}

int32_t DistributedHardwareManagerFactory::SendOnLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    int32_t ret = pthread_setname_np(pthread_self(), SEND_ONLINE);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SendOnLineEvent setname failed.");
    }
    if (networkId.size() == 0 || networkId.size() > MAX_ID_LEN || uuid.size() == 0 || uuid.size() > MAX_ID_LEN) {
        DHLOGE("NetworkId or uuid is invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGW("device is already online, uuid = %{public}s", GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE;
    }

    DHContext::GetInstance().AddOnlineDevice(uuid, networkId);

    if (!isInit && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId, uuid, deviceType);
    if (onlineResult != DH_FWK_SUCCESS) {
        DHLOGE("online failed, errCode = %{public}d", onlineResult);
        return onlineResult;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::SendOffLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty() || networkId.size() > MAX_ID_LEN || uuid.empty() || uuid.size() > MAX_ID_LEN) {
        DHLOGE("NetworkId or uuid is invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (!isInit && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGE("Device not online, networkId: %{public}s, uuid: %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE;
    }

    auto offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId, uuid, deviceType);
    if (offlineResult != DH_FWK_SUCCESS) {
        DHLOGE("offline failed, errCode = %{public}d", offlineResult);
        return offlineResult;
    }

    DHContext::GetInstance().RemoveOnlineDevice(uuid);
    if (DistributedHardwareManager::GetInstance().GetOnLineCount() == 0) {
        DHLOGI("all devices are offline, start to free the resource");
        UnInit();
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::SendDeviceChangedEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty() || networkId.size() > MAX_ID_LEN || uuid.empty() || uuid.size() > MAX_ID_LEN) {
        DHLOGE("NetworkId or uuid is invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (!isInit) {
        DHLOGI("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_NOT_ONLINE;
    }
<<<<<<< HEAD
    //if device not online, no need to handle device change event
=======

>>>>>>> 4998d93c8291ecebbf5c402940f0f67c078794ac
    if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGI("Device not online, networkId: %{public}s, uuid: %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_NOT_ONLINE;
    }

    auto result = DistributedHardwareManager::GetInstance().SendDeviceChangedEvent(networkId, uuid, deviceType);
    if (result != DH_FWK_SUCCESS) {
        DHLOGE("offline failed, errCode = %{public}d", result);
        return result;
    }

    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap)
{
    DHLOGI("start");
    return DistributedHardwareManager::GetInstance().GetComponentVersion(versionMap);
}

int32_t DistributedHardwareManagerFactory::Dump(const std::vector<std::string> &argsStr, std::string &result)
{
    return DistributedHardwareManager::GetInstance().Dump(argsStr, result);
}
} // namespace DistributedHardware
} // namespace OHOS
