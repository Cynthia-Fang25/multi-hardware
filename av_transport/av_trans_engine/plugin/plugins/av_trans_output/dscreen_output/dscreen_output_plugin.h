/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSCREEN_H
#define OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSCREEN_H

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "avtrans_output_plugin.h"
#include "foundation/osal/thread/task.h"
#include "nlohmann/json.hpp"
#include "plugin_manager.h"
#include "plugin_types.h"

namespace OHOS {
namespace DistributedHardware {

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;

using json = nlohmann::json;
using AVDataCallback = std::function<void(std::shared_ptr<Plugin::Buffer>)>;

class DscreenOutputPlugin : public AvTransOutputPlugin {
public:
    explicit DscreenOutputPlugin(std::string name);
    ~DscreenOutputPlugin();

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status SetParameter(Tag tag, const ValueType &value) override;
    Status GetParameter(Tag tag, ValueType &value) override;
    Status PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset) override;
    Status SetCallback(Callback *cb) override;
    Status SetDataCallback(AVDataCallback callback) override;

private:
    Status StartOutputQueue();
    Status ControlFrameRate(const int64_t timestamp);
    void HandleData();
    void DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue);
    void ReadMasterClockFromMemory(const std::shared_ptr<Plugin::Buffer> &buffer);

private:
    std::shared_ptr<OSAL::Task> sendDisplayTask_;
    std::queue<std::shared_ptr<Plugin::Buffer>> outputBuffer_;
    std::mutex dataQueueMtx_;
    Media::OSAL::Mutex operationMutes_ {};
    State state_ {State::CREATED};
    std::map<Tag, ValueType> paramsMap_;
    Callback *eventsCb_ = nullptr;
    AVDataCallback dataCb_;
    std::condition_variable dataCond_;
    AVTransSharedMemory sharedMemory_ = AVTransSharedMemory{ 0, 0, "" };
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSCREEN_H