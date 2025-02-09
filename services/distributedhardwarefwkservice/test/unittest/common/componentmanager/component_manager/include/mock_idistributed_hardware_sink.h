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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_IDISTRIBUTED_HARDWARE_SINK_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_IDISTRIBUTED_HARDWARE_SINK_H

#include <string>

#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
class MockIDistributedHardwareSink : public IDistributedHardwareSink {
public:
    virtual ~MockIDistributedHardwareSink() {}
    int32_t InitSink(const std::string &params)
    {
        return 0;
    }
    int32_t ReleaseSink()
    {
        return 0;
    }
    int32_t SubscribeLocalHardware(const std::string &dhId, const std::string &parameter)
    {
        return 0;
    }
    int32_t UnsubscribeLocalHardware(const std::string &dhId)
    {
        return 0;
    }
    int32_t RegisterPrivacyResources(std::shared_ptr<PrivacyResourcesListener> listener)
    {
        return 0;
    }
    int32_t PauseDistributedHardware(const std::string &networkId)
    {
        return 0;
    }
    int32_t ResumeDistributedHardware(const std::string &networkId)
    {
        return 0;
    }
    int32_t StopDistributedHardware(const std::string &networkId)
    {
        return 0;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
