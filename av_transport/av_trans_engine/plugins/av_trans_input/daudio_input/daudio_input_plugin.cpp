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

#include "daudio_input_plugin.h"

#include "foundation/utils/constants.h"
#include "plugin/common/plugin_caps_builder.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<AvTransInputPlugin> DaudioInputPluginCreator(const std::string& name)
{
    return std::make_shared<DaudioInputPlugin>(name);
}

Status DaudioInputRegister(const std::shared_ptr<Register>& reg)
{
    DHLOGI("DaudioInputPlugin enter.");
    AvTransInputPluginDef definition;
    definition.name = "AVTransDaudioInputPlugin";
    definition.description = "Audio transport from daudio service";
    definition.rank = 100;
    definition.creator = DaudioInputPluginCreator;
    definition.pluginType = PluginType::AVTRANS_INPUT;

    CapabilityBuilder capBuilder;
    capBuilder.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    DiscreteCapability<uint32_t> values = {8000, 11025, 12000, 16000,
        22050, 24000, 32000, 44100, 48000, 64000, 96000};
    capBuilder.SetAudioSampleRateList(values);
    definition.outCaps.push_back(capBuilder.Build());

    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDaudioInput, LicenseType::APACHE_V2, DaudioInputRegister, [] {});

DaudioInputPlugin::DaudioInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    DHLOGI("DaudioInputPlugin ctor.");
}

DaudioInputPlugin::~DaudioInputPlugin()
{
    DHLOGI("DaudioInputPlugin dtor.");
}

Status DaudioInputPlugin::Init()
{
    DHLOGI("Init enter.");
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::Deinit()
{
    DHLOGI("Deinit enter.");
    return Reset();
}

Status DaudioInputPlugin::Reset()
{
    DHLOGI("Reset enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    tagMap_.clear();
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    DHLOGI("GetParameter enter.");
    {
        Media::OSAL::ScopedLock lock(operationMutes_);
        auto iter = tagMap_.find(tag);
        if (iter != tagMap_.end()) {
            value = iter->second;
            return Status::OK;
        }
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DaudioInputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    DHLOGI("SetParameter enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    tagMap_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status DaudioInputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    DHLOGI("PushData enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    TRUE_RETURN_V(buffer == nullptr, Status::ERROR_NULL_POINTER);

    if (buffer->IsEmpty()) {
        DHLOGE("bufferData is Empty.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    TRUE_RETURN_V(bufferMeta == nullptr, Status::ERROR_NULL_POINTER);
    if (bufferMeta->GetType() != BufferMetaType::AUDIO) {
        DHLOGE("bufferMeta is wrong.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    ++frameNumber_;
    buffer->pts = GetCurrentTime();
    bufferMeta->SetMeta(Tag::USER_FRAME_PTS, buffer->pts);
    bufferMeta->SetMeta(Tag::USER_FRAME_NUMBER, frameNumber_.load());
    DHLOGI("AddFrameInfo buffer pts: %ld, bufferLen: %d, frameNumber: %zu.", buffer->pts,
        buffer->GetMemory()->GetSize(), Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta()));
    return Status::OK;
}

Status DaudioInputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallback enter.");
    (void)cb;
    return Status::OK;
}

Status DaudioInputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback enter.");
    (void)callback;
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
