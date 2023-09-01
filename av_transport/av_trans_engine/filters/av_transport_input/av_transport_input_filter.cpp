/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "AVInputFilter"
#include "av_transport_input_filter.h"
#include "av_trans_log.h"
#include "av_trans_constants.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVInputFilter"

static AutoRegisterFilter<AVInputFilter> g_registerFilterHelper("builtin.avtransport.avinput");

AVInputFilter::AVInputFilter(const std::string& name) : FilterBase(name), plugin_(nullptr), pluginInfo_(nullptr)
{
    AVTRANS_LOGI("ctor called");
}

AVInputFilter::~AVInputFilter()
{
    AVTRANS_LOGI("dtor called");
}

std::vector<WorkMode> AVInputFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode AVInputFilter::SetParameter(int32_t key, const Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        AVTRANS_LOGE("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (plugin_ != nullptr) {
        plugin_->SetParameter(static_cast<Plugin::Tag>(key), value);
    }
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        paramsMap_[tag] = value;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::GetParameter(int32_t key, Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        AVTRANS_LOGE("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        value = paramsMap_[tag];
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Prepare()
{
    AVTRANS_LOGI("Prepare entered.");
    if (state_ != FilterState::INITIALIZED) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    state_ = FilterState::PREPARING;
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Find plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = DoConfigure();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Configure downStream fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = PreparePlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Prepare plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    state_ = FilterState::READY;
    AVTRANS_LOGI("Prepare end.");
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Start()
{
    AVTRANS_LOGI("Start");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Start()) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin start fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Stop()
{
    AVTRANS_LOGI("Stop");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ != FilterState::RUNNING && state_ != FilterState::PAUSED) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin stop fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    plugin_->Deinit();
    plugin_ = nullptr;
    state_ = FilterState::READY;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Pause()
{
    AVTRANS_LOGI("Pause");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ == FilterState::PAUSED) {
        return ErrorCode::SUCCESS;
    }
    if ((state_ != FilterState::READY) && (state_ != FilterState::RUNNING)) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    ErrorCode ret = TranslatePluginStatus(plugin_->Pause());
    if (ret != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin pause fail!");
        return ret;
    }
    state_ = FilterState::PAUSED;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Resume()
{
    AVTRANS_LOGI("Resume");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    ErrorCode ret = TranslatePluginStatus(plugin_->Resume());
    if (ret != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin resume fail!");
        return ret;
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

void AVInputFilter::InitPorts()
{
    AVTRANS_LOGI("InitPorts");
    auto outPort = std::make_shared<OutPort>(this);
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        outPorts_.push_back(outPort);
    }
}

ErrorCode AVInputFilter::FindPlugin()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    std::string mime;
    if (paramsMap_.find(Tag::MIME) == paramsMap_.end() ||
        !Plugin::Any::IsSameTypeWith<std::string>(paramsMap_[Tag::MIME])) {
        AVTRANS_LOGE("Must set mime correctly first");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    mime = Plugin::AnyCast<std::string>(paramsMap_[Tag::MIME]);
    auto nameList = PluginManager::Instance().ListPlugins(PluginType::GENERIC_PLUGIN);
    for (const std::string& name : nameList) {
        auto info = PluginManager::Instance().GetPluginInfo(PluginType::GENERIC_PLUGIN, name);
        if (info->outCaps.empty() || mime != info->outCaps[0].mime) {
            continue;
        }
        if (DoNegotiate(info->outCaps) && CreatePlugin(info) == ErrorCode::SUCCESS) {
            AVTRANS_LOGI("CreatePlugin %s success", name_.c_str());
            return ErrorCode::SUCCESS;
        }
    }
    AVTRANS_LOGI("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}

bool AVInputFilter::DoNegotiate(const CapabilitySet& outCaps)
{
    AVTRANS_LOGI("DoNegotiate start");
    if (outCaps.empty()) {
        AVTRANS_LOGE("Input plugin must have out caps");
        return false;
    }
    for (const auto& outCap : outCaps) {
        auto thisOutCap = std::make_shared<Capability>(outCap);
        AVTRANS_LOGI("thisOutCap %s", thisOutCap->mime.c_str());
        Meta upstreamParams;
        Meta downstreamParams;
        if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
            AVTRANS_LOGE("outPorts is empty or invalid!");
            return false;
        }
        if (outPorts_[0]->Negotiate(thisOutCap, capNegWithDownstream_, upstreamParams, downstreamParams)) {
            AVTRANS_LOGI("Negotiate success");
            return true;
        }
    }
    AVTRANS_LOGI("DoNegotiate end");
    return false;
}

ErrorCode AVInputFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& selectedInfo)
{
    AVTRANS_LOGI("CreatePlugin");
    if (selectedInfo == nullptr || selectedInfo->name.empty()) {
        AVTRANS_LOGE("selectedInfo is nullptr or pluginName is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (selectedInfo->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            AVTRANS_LOGI("Reuse last plugin: %s", selectedInfo->name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            AVTRANS_LOGE("Deinit last plugin: %s error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransInput, AvTransInputPlugin>(selectedInfo->name);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("PluginManager CreatePlugin %s fail", selectedInfo->name.c_str());
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    pluginInfo_ = selectedInfo;
    AVTRANS_LOGI("Create new plugin: %s success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::DoConfigure()
{
    Plugin::Meta emptyMeta;
    Plugin::Meta targetMeta;
    if (MergeMeta(emptyMeta, targetMeta) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Merge Meta fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (ConfigMeta(targetMeta) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Config Meta fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (ConfigDownStream(targetMeta) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Config DownStream fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = InitPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Init plugin fail");
        state_ = FilterState::INITIALIZED;
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    err = ConfigPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Configure plugin fail");
        state_ = FilterState::INITIALIZED;
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::MergeMeta(const Plugin::Meta& meta, Plugin::Meta& targetMeta)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (!MergeMetaWithCapability(meta, capNegWithDownstream_, targetMeta)) {
        AVTRANS_LOGE("cannot find available capability of plugin %s", pluginInfo_->name.c_str());
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigMeta(Plugin::Meta& meta)
{
    AVTRANS_LOGI("ConfigMeta start!");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (paramsMap_.find(Tag::MEDIA_TYPE) == paramsMap_.end() ||
        !Plugin::Any::IsSameTypeWith<Plugin::MediaType>(paramsMap_[Tag::MEDIA_TYPE])) {
        AVTRANS_LOGE("MEDIA_TYPE in ParamsMap is not exist!");
        return ErrorCode::ERROR_NOT_EXISTED;
    }
    auto mediaType = Plugin::AnyCast<Plugin::MediaType>(paramsMap_[Tag::MEDIA_TYPE]);
    if (mediaType == MediaType::VIDEO) {
        ConfigVideoMeta(meta);
    } else {
        ConfigAudioMeta(meta);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigVideoMeta(Plugin::Meta& meta)
{
    AVTRANS_LOGI("ConfigVideoMeta start!");
    if (paramsMap_.find(Tag::VIDEO_WIDTH) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::VIDEO_WIDTH])) {
        uint32_t width = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_WIDTH]));
        AVTRANS_LOGI("ConfigVideoMeta: VIDEO_WIDTH is %d", width);
        meta.Set<Plugin::Tag::VIDEO_WIDTH>(width);
    }
    if (paramsMap_.find(Tag::VIDEO_HEIGHT) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::VIDEO_HEIGHT])) {
        uint32_t height = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_HEIGHT]));
        AVTRANS_LOGI("ConfigVideoMeta: VIDEO_HEIGHT is %d", height);
        meta.Set<Plugin::Tag::VIDEO_HEIGHT>(height);
    }
    if (paramsMap_.find(Tag::MEDIA_BITRATE) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::MEDIA_BITRATE])) {
        int64_t mediaBitRate = Plugin::AnyCast<int>(paramsMap_[Tag::MEDIA_BITRATE]);
        AVTRANS_LOGI("ConfigVideoMeta: MEDIA_BITRATE is %ld", mediaBitRate);
        meta.Set<Plugin::Tag::MEDIA_BITRATE>(mediaBitRate);
    }
    if (paramsMap_.find(Tag::VIDEO_FRAME_RATE) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::VIDEO_FRAME_RATE])) {
        uint32_t videoFrameRate = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_FRAME_RATE]));
        AVTRANS_LOGI("ConfigVideoMeta: VIDEO_FRAME_RATE is %d", videoFrameRate);
        meta.Set<Plugin::Tag::VIDEO_FRAME_RATE>(videoFrameRate);
    }
    if (paramsMap_.find(Tag::VIDEO_BIT_STREAM_FORMAT) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<VideoBitStreamFormat>(paramsMap_[Tag::VIDEO_BIT_STREAM_FORMAT])) {
        auto videoBitStreamFormat = Plugin::AnyCast<VideoBitStreamFormat>(paramsMap_[Tag::VIDEO_BIT_STREAM_FORMAT]);
        AVTRANS_LOGI("ConfigVideoMeta: VIDEO_BIT_STREAM_FORMAT is %d", videoBitStreamFormat);
        meta.Set<Plugin::Tag::VIDEO_BIT_STREAM_FORMAT>(std::vector<VideoBitStreamFormat>{videoBitStreamFormat});
    }
    if (paramsMap_.find(Tag::VIDEO_PIXEL_FORMAT) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<VideoPixelFormat>(paramsMap_[Tag::VIDEO_PIXEL_FORMAT])) {
        auto videoPixelFormat = Plugin::AnyCast<VideoPixelFormat>(paramsMap_[Tag::VIDEO_PIXEL_FORMAT]);
        AVTRANS_LOGI("ConfigVideoMeta: VIDEO_PIXEL_FORMAT is %d", videoPixelFormat);
        meta.Set<Plugin::Tag::VIDEO_PIXEL_FORMAT>(videoPixelFormat);
    }
    return ErrorCode::SUCCESS;
}

OHOS::Media::Plugin::AudioChannelLayout AVInputFilter::TransAudioChannelLayout(int layoutPtr)
{
    const static std::pair<int, OHOS::Media::Plugin::AudioChannelLayout> mapArray[] = {
        {1, OHOS::Media::Plugin::AudioChannelLayout::MONO},
        {2, OHOS::Media::Plugin::AudioChannelLayout::STEREO},
    };
    for (const auto& item : mapArray) {
        if (item.first == layoutPtr) {
            return item.second;
        }
    }
    return OHOS::Media::Plugin::AudioChannelLayout::UNKNOWN;
}

OHOS::Media::Plugin::AudioSampleFormat AVInputFilter::TransAudioSampleFormat(int sampleFormat)
{
    const static std::pair<int, OHOS::Media::Plugin::AudioSampleFormat> mapArray[] = {
        {0, OHOS::Media::Plugin::AudioSampleFormat::U8},
        {1, OHOS::Media::Plugin::AudioSampleFormat::S16},
        {2, OHOS::Media::Plugin::AudioSampleFormat::S24},
        {3, OHOS::Media::Plugin::AudioSampleFormat::S32},
        {4, OHOS::Media::Plugin::AudioSampleFormat::F32P},
        {-1, OHOS::Media::Plugin::AudioSampleFormat::NONE},
    };
    for (const auto& item : mapArray) {
        if (item.first == sampleFormat) {
            return item.second;
        }
    }
    return OHOS::Media::Plugin::AudioSampleFormat::NONE;
}

ErrorCode AVInputFilter::ConfigAudioMeta(Plugin::Meta& meta)
{
    AVTRANS_LOGI("ConfigAudioMeta start");
    if (paramsMap_.find(Tag::AUDIO_CHANNELS) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_CHANNELS])) {
        uint32_t audioChannel = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_CHANNELS]));
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_CHANNELS is %d", audioChannel);
        meta.Set<Plugin::Tag::AUDIO_CHANNELS>(audioChannel);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_RATE) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_SAMPLE_RATE])) {
        uint32_t sampleRate = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_RATE]));
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_SAMPLE_RATE is %d", sampleRate);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_RATE>(sampleRate);
    }
    if (paramsMap_.find(Tag::MEDIA_BITRATE) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::MEDIA_BITRATE])) {
        int64_t mediaBitRate = Plugin::AnyCast<int>(paramsMap_[Tag::MEDIA_BITRATE]);
        AVTRANS_LOGI("ConfigAudioMeta: MEDIA_BITRATE is %ld", mediaBitRate);
        meta.Set<Plugin::Tag::MEDIA_BITRATE>(mediaBitRate);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_FORMAT) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_SAMPLE_FORMAT])) {
        auto audioSampleFmtPtr = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_FORMAT]);
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_SAMPLE_FORMAT is %d", audioSampleFmtPtr);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_FORMAT>(TransAudioSampleFormat(audioSampleFmtPtr));
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNEL_LAYOUT) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT])) {
        auto layoutPtr = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT]);
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_CHANNEL_LAYOUT is %d", layoutPtr);
        meta.Set<Plugin::Tag::AUDIO_CHANNEL_LAYOUT>(TransAudioChannelLayout(layoutPtr));
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_PER_FRAME) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_SAMPLE_PER_FRAME])) {
        uint32_t samplePerFrame = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_PER_FRAME]));
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_SAMPLE_PER_FRAME is %d", samplePerFrame);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_PER_FRAME>(samplePerFrame);
    }
    if (paramsMap_.find(Tag::AUDIO_AAC_LEVEL) != paramsMap_.end() &&
        Plugin::Any::IsSameTypeWith<int>(paramsMap_[Tag::AUDIO_AAC_LEVEL])) {
        uint32_t aacLevel = static_cast<uint32_t>(Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_AAC_LEVEL]));
        AVTRANS_LOGI("ConfigAudioMeta: AUDIO_AAC_LEVEL is %d", aacLevel);
        meta.Set<Plugin::Tag::AUDIO_AAC_LEVEL>(aacLevel);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigDownStream(const Plugin::Meta& meta)
{
    Meta upstreamParams;
    Meta downstreamParams;
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        AVTRANS_LOGE("outPorts is empty or invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (!outPorts_[0]->Configure(std::make_shared<Plugin::Meta>(meta), upstreamParams, downstreamParams)) {
        AVTRANS_LOGE("Configure downstream fail");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::InitPlugin()
{
    AVTRANS_LOGI("InitPlugin");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    return TranslatePluginStatus(plugin_->Init());
}

ErrorCode AVInputFilter::ConfigPlugin()
{
    AVTRANS_LOGI("Configure");
    ErrorCode err = SetPluginParams();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Set Plugin fail!");
        return err;
    }
    err = SetEventCallBack();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Plugin SetEventCallBack fail!");
        return err;
    }
    err = SetDataCallBack();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Plugin SetDataCallBack fail!");
        return err;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::SetPluginParams()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (paramsMap_.find(Tag::MEDIA_DESCRIPTION) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::MEDIA_DESCRIPTION, paramsMap_[Tag::MEDIA_DESCRIPTION]);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::PreparePlugin()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return TranslatePluginStatus(plugin_->Prepare());
}

ErrorCode AVInputFilter::PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (name_.compare(inPort) != 0) {
        AVTRANS_LOGE("FilterName is not targetName!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (buffer == nullptr || plugin_ == nullptr) {
        AVTRANS_LOGE("buffer or plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    if (TranslatePluginStatus(plugin_->PushData(inPort, buffer, offset)) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("PushData to plugin fail!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }

    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        AVTRANS_LOGE("outPorts is empty or invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    outPorts_[0]->PushData(buffer, 0);
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::SetEventCallBack()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER ;
    }
    return TranslatePluginStatus(plugin_->SetCallback(this));
}

ErrorCode AVInputFilter::SetDataCallBack()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return TranslatePluginStatus(plugin_->SetDataCallback(std::bind(&AVInputFilter::OnDataCallback, this,
        std::placeholders::_1)));
}

void AVInputFilter::OnDataCallback(std::shared_ptr<Plugin::Buffer> buffer)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (buffer == nullptr) {
        AVTRANS_LOGE("buffer is nullptr!");
        return;
    }
    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        AVTRANS_LOGE("outPorts is invalid!");
        return;
    }
    outPorts_[0]->PushData(buffer, 0);
}
} // namespace DistributedHardware
} // namespace OHOS