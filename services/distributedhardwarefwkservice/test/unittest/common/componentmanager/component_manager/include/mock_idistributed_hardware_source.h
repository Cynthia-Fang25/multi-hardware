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

#ifndef OHOS_MOCK_IDISTRIBUTED_HARDWARE_SOURCE_H
#define OHOS_MOCK_IDISTRIBUTED_HARDWARE_SOURCE_H

#include <string>

#include "idistributed_hardware_source.h"
namespace OHOS {
namespace DistributedHardware {
class MockIDistributedHardwareSource : public IDistributedHardwareSource {
public:
    virtual ~MockIDistributedHardwareSource() {}
    int32_t InitSource(const std::string &params)
    {
        return 0;
    }
    int32_t ReleaseSource()
    {
        return 0;
    }
    int32_t RegisterDistributedHardware(const std::string &uuid, const std::string &dhId,
        const EnableParam &parameters, std::shared_ptr<RegisterCallback> callback)
    {
        return 0;
    }
    int32_t UnregisterDistributedHardware(const std::string &uuid, const std::string &dhId,
        std::shared_ptr<UnregisterCallback> callback)
    {
        return 0;
    }
    int32_t ConfigDistributedHardware(const std::string &uuid, const std::string &dhId, const std::string &key,
        const std::string &value)
    {
        return 0;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
