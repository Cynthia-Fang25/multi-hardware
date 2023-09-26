/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_HIDUMP_HELPER_H
#define OHOS_DISTRIBUTED_HARDWARE_HIDUMP_HELPER_H
#include <cstdint>
#include <set>
#include <string>

#include "enabled_comps_dump.h"
#include "device_type.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
enum class HidumpFlag {
    UNKNOWN = 0,
    GET_HELP,
    GET_LOADED_COMP_LIST,
    GET_ENABLED_COMP_LIST,
    GET_TASK_LIST,
    GET_CAPABILITY_LIST,
};
constexpr uint32_t DUMP_FILE_MAX_SIZE = 295 * 1024 * 1024;

class HidumpHelper {
DECLARE_SINGLE_INSTANCE_BASE(HidumpHelper);
public:
    int32_t Dump(const std::vector<std::string>& args, std::string &result);
    bool GetDumpFlag();
    bool GetFileFullFlag();
    bool GetReDumpFlag();
    void SetDumpFlagTrue();
    void SetDumpFlagFalse();
    void SetFileFullFlagTrue();
    void SetFileFullFlagFalse();
    void SetReDumpFlagTrue();
    void SetReDumpFlagFalse();
    void DumpDfxDataToFile(std::string fileName, uint8_t *buffer, int32_t bufSize);

private:
    explicit HidumpHelper() = default;
    ~HidumpHelper() = default;

    bool DumpFlag_ = false;
    bool FileFullFlag_ = false;
    bool ReDumpFlag_ = false;

    int32_t DumpScreenData(std::string &result);
    int32_t ReDumpScreenData(std::string &result);
    int32_t ProcessDump(const HidumpFlag &flag, std::string &result);
    int32_t ShowAllLoadedComps(std::string &result);
    int32_t ShowAllEnabledComps(std::string &result);
    int32_t ShowAllTaskInfos(std::string &result);
    int32_t ShowAllCapabilityInfos(std::string &result);
    int32_t ShowHelp(std::string &result);
    int32_t ShowIllealInfomation(std::string &result);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif