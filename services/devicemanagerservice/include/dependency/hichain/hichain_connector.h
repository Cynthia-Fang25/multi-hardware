/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_HICHAIN_CONNECTOR_H
#define OHOS_HICHAIN_CONNECTOR_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "device_auth.h"
#include "hichain_connector_callback.h"
#include "nlohmann/json.hpp"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;

    GroupInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0)
    {
    }
};

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo);

class HiChainConnector {
public:
    static bool onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    static void onFinish(int64_t requestId, int32_t operationCode, const char *returnData);
    static void onError(int64_t requestId, int32_t operationCode, int32_t errorCode, const char *errorReturn);
    static char *onRequest(int64_t requestId, int32_t operationCode, const char *reqParams);

public:
    HiChainConnector();
    ~HiChainConnector();
    int32_t RegisterHiChainCallback(const std::string &pkgName, std::shared_ptr<IHiChainConnectorCallback> callback);
    int32_t UnRegisterHiChainCallback(const std::string &pkgName);
    int32_t CreateGroup(int64_t requestId, const std::string &groupName);
    int32_t AddMember(std::string deviceId, std::string &connectInfo);
    int32_t DelMemberFromGroup(std::string groupId, std::string deviceId);
    void DeleteGroup(std::string &groupId);
    bool IsDevicesInGroup(std::string hostDevice, std::string peerDevice);
    void GetRelatedGroups(std::string DeviceId, std::vector<GroupInfo> &groupList);

private:
    int64_t GenRequestId();
    void SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList);
    void GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList);
    int32_t GetGroupInfo(std::string queryParams, std::vector<GroupInfo> &groupList);
    std::string GetConnectPara(std::string deviceId, std::string reqDeviceId);
    bool IsGroupCreated(std::string groupName, GroupInfo &groupInfo);
    bool IsGroupInfoInvalid(GroupInfo &group);

private:
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
    DeviceAuthCallback deviceAuthCallback_;
    static std::map<std::string, std::shared_ptr<IHiChainConnectorCallback>> hiChainConnectorCallbackMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
