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

#include "task_fuzzer.h"

#include <cstddef>
#include <cstdint>


#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "impl_utils.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const uint32_t DH_TYPE_SIZE = 10;
    const DHType dhTypeFuzz[DH_TYPE_SIZE] = {
        DHType::CAMERA, DHType::MIC, DHType::SPEAKER, DHType::DISPLAY, DHType::VIRMODEM_MIC,
        DHType::INPUT, DHType::A2D, DHType::GPS, DHType::HFP, DHType::VIRMODEM_SPEAKER
    };

    const uint32_t TASK_TYPE_SIZE = 5;
    const TaskType taskTypeFuzz[TASK_TYPE_SIZE] = {
        TaskType::UNKNOWN = 0, TaskType::ENABLE = 1, TaskType::DISABLE = 2, TaskType::ON_LINE = 3, TaskType::OFF_LINE = 4
    };
}

void TaskFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::string uuid(reinterpret_cast<const char*>(data), size);
    std::string dhId(reinterpret_cast<const char*>(data), size);
    DHType dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];

    TaskParam taskParam = {
        .networkId = networkId,
        .uuid = uuid,
        .dhId = dhId,
        .dhType = dhType
    };
    TaskType taskType = taskTypeFuzz[data[0] % TASK_TYPE_SIZE];

    auto task = TaskFactory::GetInstance().CreateTask(taskType, taskParam, nullptr);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::TaskFuzzTest(data, size);
    return 0;
}

