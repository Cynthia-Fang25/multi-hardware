/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_NOTIFY_DMFA_RESULT_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_NOTIFY_DMFA_RESULT_REQ_H

#include <stdint.h>

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyDMFAResultReq : public IpcReq {
DECLARE_IPC_MODEL(IpcNotifyDMFAResultReq);
public:
    std::string GetPkgName() const
    {
        return PackageName_;
    }

    void SetPkgName(std::string& pkgName)
    {
        PackageName_ = pkgName;
    }

    std::string GetJsonParam() const
    {
        return JsonParam_;
    }

    void SetJsonParam(std::string& JsonParam)
    {
        JsonParam_ = JsonParam;
    }
private:
    std::string PackageName_;
    std::string JsonParam_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_NOTIFY_AUTH_RESULT_REQ_H
