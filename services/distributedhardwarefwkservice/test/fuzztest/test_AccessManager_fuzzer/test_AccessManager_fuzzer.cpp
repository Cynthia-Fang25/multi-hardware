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

#include "test_AccessManager_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <thread>

#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"

constexpr int32_t INTERVAL_TIME_MS = 100;
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
constexpr uint16_t STR_LEN = 32;

namespace OHOS {
namespace DistributedHardware {
bool SendOnOffLineEvent(const uint8_t* data, size_t size)
{
    if (size > sizeof(uint32_t) + STR_LEN * 2) {
        std::string networkId(reinterpret_cast<const char*>(data), STR_LEN);
        std::string uuid(reinterpret_cast<const char*>(data + STR_LEN), STR_LEN);

        auto ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(
            networkId, uuid, TEST_DEV_TYPE_PAD);
        if (ret != DH_FWK_SUCCESS) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

        ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(
            networkId, uuid, TEST_DEV_TYPE_PAD);

        return (ret == DH_FWK_SUCCESS);
    }
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SendOnOffLineEvent(data, size);
    return 0;
}

