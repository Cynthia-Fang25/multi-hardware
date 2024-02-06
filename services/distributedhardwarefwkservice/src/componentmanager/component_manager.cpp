/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "component_manager.h"

#include <cinttypes>
#include <future>
#include <pthread.h>
#include <string>
#include <thread>

#include "ipc_object_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "constants.h"
#include "device_manager.h"
#include "dh_context.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "enabled_comps_dump.h"
#include "low_latency.h"
#include "publisher.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManager"

IMPLEMENT_SINGLE_INSTANCE(ComponentManager);

namespace {
    constexpr int32_t ENABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t DISABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t ENABLE_PARAM_RETRY_TIME = 500 * 1000;
    constexpr int32_t INVALID_SA_ID = -1;
    constexpr int32_t MONITOR_TASK_DELAY_MS = 5 * 1000;
    constexpr int32_t UNINIT_COMPONENT_TIMEOUT_SECONDS = 2;
    const std::string MONITOR_TASK_TIMER_ID = "monitor_task_timer_id";
}

ComponentManager::ComponentManager() : compSource_({}), compSink_({}), compSrcSaId_({}),
    compMonitorPtr_(std::make_shared<ComponentMonitor>()), lowLatencyListener_(new(std::nothrow) LowLatencyListener),
    monitorTaskTimer_(std::make_shared<MonitorTaskTimer>(MONITOR_TASK_TIMER_ID, MONITOR_TASK_DELAY_MS)),
    isUnInitTimeOut_(false)
{
    DHLOGI("Ctor ComponentManager");
}

ComponentManager::~ComponentManager()
{
    DHLOGD("Dtor ComponentManager");
    compMonitorPtr_.reset();
    compMonitorPtr_ = nullptr;
    lowLatencyListener_ = nullptr;
}

int32_t ComponentManager::Init()
{
    DHLOGI("start.");
    DHTraceStart(COMPONENT_INIT_START);
    if (!InitCompSource()) {
        DHLOGE("InitCompSource failed.");
        DHTraceEnd();
        return ERR_DH_FWK_COMPONENT_INIT_SOURCE_FAILED;
    }
    if (!InitCompSink()) {
        DHLOGE("InitCompSink failed.");
        compSource_.clear();
        DHTraceEnd();
        return ERR_DH_FWK_COMPONENT_INIT_SINK_FAILED;
    }

    if (compMonitorPtr_ == nullptr) {
        DHLOGE("compMonitorPtr_ is null.");
        return ERR_DH_FWK_COMPONENT_MONITOR_NULL;
    }
    for (const auto &comp : compSource_) {
        if (compSrcSaId_.find(comp.first) == compSrcSaId_.end()) {
            continue;
        }
        compMonitorPtr_->AddSAMonitor(compSrcSaId_.at(comp.first));
    }

    auto sourceResult = StartSource();
    auto sinkResult = StartSink();

    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("StartSource failed, some virtual components maybe cannot work, but want to continue");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start source failed.");
    }
    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("StartSink failed, some virtual components maybe cannot work, but want to continue");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start sink failed.");
    }
    if (monitorTaskTimer_ != nullptr) {
        monitorTaskTimer_->StartTimer();
    }
#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().RegisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
#endif
    DHLOGI("Init component success");
    DHTraceEnd();
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UnInit()
{
    DHLOGI("start.");
    if (compMonitorPtr_ == nullptr) {
        DHLOGE("compMonitorPtr_ is null.");
        return ERR_DH_FWK_COMPONENT_MONITOR_NULL;
    }
    for (const auto &comp : compSource_) {
        if (compSrcSaId_.find(comp.first) == compSrcSaId_.end()) {
            continue;
        }
        compMonitorPtr_->RemoveSAMonitor(compSrcSaId_.at(comp.first));
    }
    auto sourceResult = StopSource();
    auto sinkResult = StopSink();

    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }
    if (!WaitForResult(Action::STOP_SINK, sinkResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }

    compSource_.clear();
    compSink_.clear();

    if (monitorTaskTimer_ != nullptr) {
        monitorTaskTimer_->StopTimer();
    }
#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().UnregisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
    LowLatency::GetInstance().CloseLowLatency();
#endif
    DHLOGI("Release component success");

    if (isUnInitTimeOut_.load()) {
        DHLOGE("Some component stop timeout, FORCE exit!");
        _Exit(0);
    }

    return DH_FWK_SUCCESS;
}

ActionResult ComponentManager::StartSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSource_) {
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sourceVersion;
        auto future = std::async(std::launch::async, [item, params]() { return item.second->InitSource(params); });
        futures.emplace(item.first, future.share());
    }
    return futures;
}

ActionResult ComponentManager::StartSource(DHType dhType)
{
    DHLOGI("Start Source, dhType: %" PRIu32, (uint32_t)dhType);
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("Component for DHType: %" PRIu32 " not init source handler", (uint32_t)dhType);
        return futures;
    }

    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sourceVersion;
    auto future = std::async(std::launch::async, [this, dhType, params]() {
        return compSource_[dhType]->InitSource(params);
    });
    futures.emplace(dhType, future.share());

    return futures;
}

ActionResult ComponentManager::StartSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSink_) {
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sinkVersion;
        auto future = std::async(std::launch::async, [item, params]() { return item.second->InitSink(params); });
        futures.emplace(item.first, future.share());
        if (cameraCompPrivacy_ == nullptr && item.first == DHType::CAMERA) {
            cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
            item.second->RegisterPrivacyResources(cameraCompPrivacy_);
        }
        if (audioCompPrivacy_ == nullptr && item.first == DHType::AUDIO) {
            audioCompPrivacy_ = std::make_shared<ComponentPrivacy>();
            item.second->RegisterPrivacyResources(audioCompPrivacy_);
        }
    }
    return futures;
}

ActionResult ComponentManager::StartSink(DHType dhType)
{
    DHLOGI("Start Sink, dhType: %" PRIu32, (uint32_t)dhType);
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    if (compSink_.find(dhType) == compSink_.end()) {
        DHLOGE("Component for DHType: %" PRIu32 " not init sink handler", (uint32_t)dhType);
        return futures;
    }

    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sinkVersion;
    auto future = std::async(std::launch::async, [this, dhType, params]() {
        return compSink_[dhType]->InitSink(params);
    });
    futures.emplace(dhType, future.share());
    if (cameraCompPrivacy_ == nullptr && dhType == DHType::CAMERA) {
        cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(cameraCompPrivacy_);
    }
    if (audioCompPrivacy_ == nullptr && dhType == DHType::AUDIO) {
        audioCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(audioCompPrivacy_);
    }

    return futures;
}

ActionResult ComponentManager::StopSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSource_) {
        auto future = std::async(std::launch::async, [item]() { return item.second->ReleaseSource(); });
        futures.emplace(item.first, future.share());
    }
    return futures;
}

ActionResult ComponentManager::StopSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSink_) {
        auto future = std::async(std::launch::async, [item]() {
            int32_t status = item.second->ReleaseSink();
            IHardwareHandler *hardwareHandler = nullptr;
            status = ComponentLoader::GetInstance().GetHardwareHandler(item.first, hardwareHandler);
            if (status != DH_FWK_SUCCESS || hardwareHandler == nullptr) {
                DHLOGE("GetHardwareHandler %#X failed", item.first);
                return status;
            }
            hardwareHandler->UnRegisterPluginListener();
            return status;
        });

        futures.emplace(item.first, future.share());
    }
    return futures;
}

bool ComponentManager::WaitForResult(const Action &action, ActionResult actionsResult)
{
    DHLOGD("start.");
    auto ret = true;
    for (auto &iter : actionsResult) {
        std::future_status status = iter.second.wait_for(std::chrono::seconds(UNINIT_COMPONENT_TIMEOUT_SECONDS));
        if (status == std::future_status::ready) {
            auto result = iter.second.get();
            DHLOGI("action = %d, compType = %#X, READY, ret = %d.", static_cast<int32_t>(action), iter.first, result);
            if (result != DH_FWK_SUCCESS) {
                ret = false;
                DHLOGE("there is error, but want to continue.");
            }
        }

        if (status == std::future_status::timeout) {
            DHLOGI("action = %d, compType = %#X, TIMEOUT", static_cast<int32_t>(action), iter.first);
            if (action == Action::STOP_SOURCE || action == Action::STOP_SINK) {
                isUnInitTimeOut_ = true;
            }
        }

        if (status == std::future_status::deferred) {
            DHLOGI("action = %d, compType = %#X, DEFERRED", static_cast<int32_t>(action), iter.first);
        }
    }
    DHLOGD("end.");
    return ret;
}

bool ComponentManager::InitCompSource()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(type, sourcePtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSource failed, compType = %#X, ret = %d.", type, ret);
            continue;
        }
        if (sourcePtr == nullptr) {
            DHLOGW("sourcePtr is null, compType = %#X.", type);
            continue;
        }
        compSource_.insert(std::make_pair(type, sourcePtr));

        int32_t saId = ComponentLoader::GetInstance().GetSourceSaId(type);
        if (saId != INVALID_SA_ID) {
            compSrcSaId_.insert(std::make_pair(type, saId));
        }
    }
    return !compSource_.empty();
}

bool ComponentManager::InitCompSink()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSink *sinkPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSink(type, sinkPtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSink failed, compType = %#X, ret = %d.", type, ret);
            continue;
        }
        if (sinkPtr == nullptr) {
            DHLOGW("sinkPtr is null, compType = %#X.", type);
            continue;
        }
        compSink_.insert(std::make_pair(type, sinkPtr));
    }
    return !compSink_.empty();
}

int32_t ComponentManager::Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    DHLOGI("start.");
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    EnableParam param;
    auto ret = GetEnableParam(networkId, uuid, dhId, dhType, param);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetEnableParam failed, uuid = %s, dhId = %s, errCode = %d", GetAnonyString(uuid).c_str(),
            GetAnonyString(dhId).c_str(), ret);
        if (RetryGetEnableParam(networkId, uuid, dhId, dhType, param) != DH_FWK_SUCCESS) {
            return ret;
        }
    }
    std::string subtype = param.subtype;
    std::map<std::string, bool> resourceDesc = ComponentLoader::GetInstance().GetCompResourceDesc();
    if (resourceDesc.find(subtype) == resourceDesc.end()) {
        DHLOGE("GetCompResourceDesc failed.");
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    bool sensitiveVal = resourceDesc[subtype];
    bool isSameAuthForm = IsIdenticalAccount(networkId);
    if (sensitiveVal && !isSameAuthForm) {
        DHLOGE("Privacy resources must be logged in with the same account.");
        return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
    }

    auto compEnable = std::make_shared<ComponentEnable>();
    auto result = compEnable->Enable(networkId, dhId, param, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
            if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already offline, no need try enable, uuid = %s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compEnable->Enable(networkId, dhId, param, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("enable success, retryCount = %d", retryCount);
                EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("enable failed, retryCount = %d", retryCount);
        }
        return result;
    }
    DHLOGI("enable result is %d, uuid = %s, dhId = %s", result, GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);

    return result;
}

int32_t ComponentManager::RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, const DHType dhType, EnableParam &param)
{
    for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
        if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
            DHLOGE("device is already offline, no need try GetEnableParam, uuid = %s",
                GetAnonyString(uuid).c_str());
            return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
        }
        if (GetEnableParam(networkId, uuid, dhId, dhType, param) == DH_FWK_SUCCESS) {
            DHLOGE("GetEnableParam success, retryCount = %d", retryCount);
            break;
        }
        DHLOGE("GetEnableParam failed, retryCount = %d", retryCount);
        usleep(ENABLE_PARAM_RETRY_TIME);
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }

    auto compDisable = std::make_shared<ComponentDisable>();
    auto result = compDisable->Disable(networkId, dhId, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < DISABLE_RETRY_MAX_TIMES; retryCount++) {
            if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already online, no need try disable, uuid = %s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compDisable->Disable(networkId, dhId, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("disable success, retryCount = %d", retryCount);
                EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("disable failed, retryCount = %d", retryCount);
        }
        return result;
    }
    DHLOGI("disable result is %d, uuid = %s, dhId = %s", result, GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);

    return result;
}

DHType ComponentManager::GetDHType(const std::string &uuid, const std::string &dhId) const
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        return capability->GetDHType();
    }
    DHLOGE("get dhType failed, uuid = %s, dhId = %s", GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    return DHType::UNKNOWN;
}

int32_t ComponentManager::GetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, DHType dhType, EnableParam &param)
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret != DH_FWK_SUCCESS) || (capability == nullptr)) {
        DHLOGE("GetCapability failed, uuid =%s, dhId = %s, errCode = %d", GetAnonyString(uuid).c_str(),
            GetAnonyString(dhId).c_str(), ret);
        return ret;
    }
    DeviceInfo sourceDeviceInfo = GetLocalDeviceInfo();
    std::string sourceNetworkId = DHContext::GetInstance().GetNetworkIdByUUID(sourceDeviceInfo.uuid);
    std::vector<std::shared_ptr<CapabilityInfo>> sourceCapInfos;
    std::string sourceDHId;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(sourceDeviceInfo.deviceId, sourceCapInfos);
    for (const auto &capInfo : sourceCapInfos) {
        if (dhType == capInfo->GetDHType()) {
            param.sourceAttrs = capInfo->GetDHAttrs();
            sourceDHId = capInfo->GetDHId();
        }
    }
    std::string sourceVersion("");
    ret = GetVersion(sourceNetworkId, sourceDeviceInfo.uuid, dhType, sourceVersion, false);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get source version failed, uuid = %s, dhId = %s, dhType = %#X,",
            GetAnonyString(sourceDeviceInfo.uuid).c_str(), GetAnonyString(sourceDHId).c_str(), dhType);
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sourceVersion = sourceVersion;
    param.sinkAttrs = capability->GetDHAttrs();
    std::string sinkVersion("");
    ret = GetVersion(networkId, uuid, dhType, sinkVersion, true);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get sink version failed, uuid = %s, dhId = %s, dhType = %#X,", GetAnonyString(uuid).c_str(),
            GetAnonyString(dhId).c_str(), dhType);
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sinkVersion = sinkVersion;
    param.subtype = capability->GetDHSubtype();
    DHLOGI("success. dhType = %#X, sink uuid =%s, sink dhId = %s, sinVersion = %s, source uuid =%s, source dhId = %s, "
        "sourceVersion = %s, subtype = %s", dhType, GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(),
        param.sinkVersion.c_str(), GetAnonyString(sourceDeviceInfo.uuid).c_str(),
        GetAnonyString(sourceDHId).c_str(), param.sourceVersion.c_str(), param.subtype.c_str());

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersionFromVerMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    CompVersion compversion;
    int32_t ret = VersionManager::GetInstance().GetCompVersion(uuid, dhType, compversion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get version Manager failed, uuid =%s, dhType = %#X, errCode = %d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    DHLOGI("Get version mgr success, sinkVersion = %s, sourceVersion = %s,uuid = %s, dhType = %#X",
        compversion.sinkVersion.c_str(), compversion.sourceVersion.c_str(), GetAnonyString(uuid).c_str(), dhType);
    version = isSink ? compversion.sinkVersion : compversion.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    VersionInfo versionInfo;
    int32_t ret =  VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(GetDeviceIdByUUID(uuid), versionInfo);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get Version info Manager failed, uuid =%s, dhType = %#X, errCode = %d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    auto iter = versionInfo.compVersions.find(dhType);
    if (iter == versionInfo.compVersions.end()) {
        DHLOGE("can not find component version for dhType = %d", dhType);
        return ERR_DH_FWK_COMPONENT_DHTYPE_NOT_FOUND;
    }
    DHLOGI("Get version info mgr success, sinkVersion = %s, sourceVersion = %s, uuid = %s, dhType = %#X",
        iter->second.sinkVersion.c_str(), iter->second.sourceVersion.c_str(), GetAnonyString(uuid).c_str(), dhType);
    UpdateVersionCache(uuid, versionInfo);
    version = isSink ? iter->second.sinkVersion : iter->second.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersion(const std::string &networkId, const std::string &uuid,
    DHType dhType, std::string &version, bool isSink)
{
    int32_t ret = GetVersionFromVerMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    ret = GetVersionFromVerInfoMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    return ret;
}

void ComponentManager::UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo)
{
    DHVersion dhVersion;
    dhVersion.uuid = uuid;
    dhVersion.dhVersion = versionInfo.dhVersion;
    dhVersion.compVersions = versionInfo.compVersions;
    VersionManager::GetInstance().AddDHVersion(uuid, dhVersion);
}

void ComponentManager::DumpLoadedComps(std::set<DHType> &compSourceType, std::set<DHType> &compSinkType)
{
    for (auto compSource : compSource_) {
        compSourceType.emplace(compSource.first);
    }
    for (auto compSink : compSink_) {
        compSinkType.emplace(compSink.first);
    }
}

void ComponentManager::Recover(DHType dhType)
{
    std::thread(&ComponentManager::DoRecover, this, dhType).detach();
}

void ComponentManager::DoRecover(DHType dhType)
{
    int32_t ret = pthread_setname_np(pthread_self(), DO_RECOVER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoRecover setname failed.");
    }
    // step1: restart sa process
    ReStartSA(dhType);
    // step2: recover distributed hardware virtual driver
    RecoverDistributedHardware(dhType);
}

void ComponentManager::ReStartSA(DHType dhType)
{
    DHLOGI("Restart SA for DHType %" PRIu32, (uint32_t)dhType);
    auto sourceResult = StartSource(dhType);
    auto sinkResult = StartSink(dhType);

    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("ReStartSource failed, DHType: %" PRIu32, (uint32_t)dhType);
    }

    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("ReStartSink failed, DHType: %" PRIu32, (uint32_t)dhType);
    }
    DHLOGI("Finish Restart");
}

void ComponentManager::RecoverDistributedHardware(DHType dhType)
{
    CapabilityInfoMap capabilityMap;
    CapabilityInfoManager::GetInstance()->GetDataByDHType(dhType, capabilityMap);
    for (const auto &capInfo : capabilityMap) {
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capInfo.second->GetDeviceId());
        if (uuid.empty()) {
            DHLOGE("Can not find uuid by capability deviceId: %s",
                GetAnonyString(capInfo.second->GetDeviceId()).c_str());
            continue;
        }

        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Can not find network id by uuid: %s", GetAnonyString(uuid).c_str());
            continue;
        }

        TaskParam taskParam = {
            .networkId = networkId,
            .uuid = uuid,
            .dhId = capInfo.second->GetDHId(),
            .dhType = capInfo.second->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
}

std::map<DHType, IDistributedHardwareSink*> ComponentManager::GetDHSinkInstance()
{
    return compSink_;
}

bool ComponentManager::IsIdenticalAccount(const std::string &networkId)
{
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("DeviceList size is invalid!");
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            authForm = deviceInfo.authForm;
            break;
        }
    }
    if (authForm == DmAuthForm::IDENTICAL_ACCOUNT) {
        return true;
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
