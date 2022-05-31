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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHUTIL_HISYSEVENT_H
#define OHOS_DISTRIBUTED_HARDWARE_DHUTIL_HISYSEVENT_H

#include <string>

#include "hisysevent.h"

#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DHFWK_SA_START = "DHFWK_SA_START";
const std::string DHFWK_SA_STOP = "DHFWK_SA_STOP";
const std::string COMP_LOAD = "COMP_LOAD";
const std::string FILE_PARSE_ERROR = "FILE_PARSE_ERROR";
const std::string COMP_SO_LOAD_ERROR = "COMP_SO_LOAD_ERROR";
const std::string COMP_RELEASE = "COMP_RELEASE";
const std::string COMP_RELEASE_ERROR = "COMP_RELEASE_ERROR";
const std::string ON_LINE_TASK = "ON_LINE_TASK";
const std::string OFF_LINE_TASK = "OFF_LINE_TASK";
const std::string ENABLE_TASK = "ENABLE_TASK";
const std::string DISABLE_TASK = "DISABLE_TASK";
const std::string ENABLE_FAILED = "ENABLE_FAILED";
const std::string DISABLE_FAILED = "DISABLE_FAILED";
const std::string DB_DATA_NOTIFY = "DB_DATA_NOTIFY";

void HiSysEventWriteMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &msg);
void HiSysEventWriteCompLoadMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &soName, const std::string &msg);
void HiSysEventWriteCompReleaseMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const DHType dhType, int32_t ret, const std::string &msg);
void HiSysEventWriteOnOffLineMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDevid, const std::string &anonyUUID, const std::string &msg);
void HiSysEventWriteAbleTaskMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDevid, const std::string &anonyDHId, const std::string &msg);
void HiSysEventWriteAbleFailedMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDHId, int32_t ret,  const std::string &msg);
void HiSysEventWriteDBDataNotifyMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDevid, const std::string &anonyDHId, const std::string &msg);
} // namespace DistributedHardware
} // namespace OHOS
#endif