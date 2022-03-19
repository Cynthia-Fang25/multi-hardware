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

#ifndef OHOS_DEVICE_MANAGER_GET_LOCAL_DEVICE_INFO_RSP_H
#define OHOS_DEVICE_MANAGER_GET_LOCAL_DEVICE_INFO_RSP_H

#include "dm_device_info.h"
#include "message_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class GetLocalDeviceInfoRsp : public MessageRsp {
    DECLARE_MESSAGE_MODEL(GetLocalDeviceInfoRsp);

public:
    const DmDeviceInfo &GetLocalDeviceInfo() const
    {
        return localDeviceInfo_;
    }

    void SetLocalDeviceInfo(DmDeviceInfo &localDeviceInfo)
    {
        localDeviceInfo_ = localDeviceInfo;
    }

private:
    DmDeviceInfo localDeviceInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_GET_LOCAL_DEVICE_INFO_RSP_H
