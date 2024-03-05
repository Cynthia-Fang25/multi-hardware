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

#include "av_trans_meta.h"

#include "av_trans_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
const std::string META_DATA_TYPE = "meta_data_type";
const std::string META_TIMESTAMP = "meta_timestamp";
const std::string META_FRAME_NUMBER = "meta_frame_number";
const std::string META_EXT_TIMESTAMP = "meta_ext_timestamp";
const std::string META_EXT_FRAME_NUMBER = "meta_ext_frame_number";

std::shared_ptr<OHOS::Media::Plugin::BufferMeta> AVTransAudioBufferMeta::Clone()
{
    auto bufferMeta = std::make_shared<AVTransAudioBufferMeta>();
    bufferMeta->pts_ = pts_;
    bufferMeta->cts_ = cts_;
    bufferMeta->format_ = format_;
    bufferMeta->dataType_ = dataType_;
    bufferMeta->frameNum_ = frameNum_;
    bufferMeta->channels_ = channels_;
    bufferMeta->sampleRate_ = sampleRate_;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::string AVTransAudioBufferMeta::MarshalAudioMeta()
{
    // nlohmann::json metaJson;
    // metaJson[META_DATA_TYPE] = dataType_;
    // metaJson[META_TIMESTAMP] = pts_;
    // metaJson[META_FRAME_NUMBER] = frameNum_;
    // return metaJson.dump();
    cJSON *metaJson = cJSON_CreateObject();
    if (metaJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(metaJson, META_DATA_TYPE.c_str(), dataType_);
    cJSON_AddNumberToObject(metaJson, META_TIMESTAMP.c_str(), pts_);
    cJSON_AddNumberToObject(metaJson, META_FRAME_NUMBER.c_str(), frameNum_);
    char *jsonstr = cJSON_Print(metaJson);
    if (jsonstr == nullptr) {
        cJSON_Delete(metaJson);
        return "";
    }
    cJSON_Delete(metaJson);
    cJSON_free(jsonstr);
    return std::string(jsonstr);
}

bool AVTransAudioBufferMeta::UnmarshalAudioMeta(const std::string& jsonStr)
{
    // nlohmann::json metaJson = nlohmann::json::parse(jsonStr, nullptr, false);
    // if (metaJson.is_discarded()) {
    //     return false;
    // }
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr || !cJSON_IsObject(metaJson)) {
        return ;
    }
    if (!IsUInt32(metaJson, META_DATA_TYPE) || !IsInt64(metaJson, META_TIMESTAMP) ||
        !IsUInt32(metaJson, META_FRAME_NUMBER)) {
        return false;
    }
    dataType_ = metaJson[META_DATA_TYPE].get<BufferDataType>();
    // pts_ = metaJson[META_TIMESTAMP].get<int64_t>();
    // frameNum_ = metaJson[META_FRAME_NUMBER].get<uint32_t>();
    cJSON *typeObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_DATA_TYPE.c_str());
    if (typeObj == nullptr || !cJSON_IsNumber(typeObj)) {
        return false;
    }
    cJSON *ptsObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP.c_str());
    if (ptsObj == nullptr || !cJSON_IsNumber(ptsObj)) {
        return false;
    }
    cJSON *frameObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_FRAME_NUMBER.c_str());
    if (frameObj == nullptr || !cJSON_IsNumber(frameObj)) {
        return false;
    }
    dataType_ = typeObj->valueint;
    pts_ = ptsObj->valueint;
    frameNum_ = frameObj->valueint;
    return true;
}

std::shared_ptr<OHOS::Media::Plugin::BufferMeta> AVTransVideoBufferMeta::Clone()
{
    auto bufferMeta = std::make_shared<AVTransVideoBufferMeta>();
    bufferMeta->pts_ = pts_;
    bufferMeta->cts_ = cts_;
    bufferMeta->width_ = width_;
    bufferMeta->height_ = height_;
    bufferMeta->format_ = format_;
    bufferMeta->dataType_ = dataType_;
    bufferMeta->frameNum_ = frameNum_;
    bufferMeta->extPts_ = extPts_;
    bufferMeta->extFrameNum_ = extFrameNum_;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::string AVTransVideoBufferMeta::MarshalVideoMeta()
{
    // nlohmann::json metaJson;
    // metaJson[META_DATA_TYPE] = dataType_;
    // metaJson[META_TIMESTAMP] = pts_;
    // metaJson[META_FRAME_NUMBER] = frameNum_;
    // if (extPts_ > 0) {
    //     metaJson[META_EXT_TIMESTAMP] = extPts_;
    // }
    // if (extFrameNum_ > 0) {
    //     metaJson[META_EXT_FRAME_NUMBER] = extFrameNum_;
    // }
    // return metaJson.dump();
    cJSON *metaJson = cJSON_CreateObject();
    if (metaJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(metaJson, META_DATA_TYPE.c_str(), dataType_);
    cJSON_AddNumberToObject(metaJson, META_TIMESTAMP.c_str(), pts_);
    cJSON_AddNumberToObject(metaJson, META_FRAME_NUMBER.c_str(), frameNum_);
    if (extPts_ > 0) {
        //metaJson[META_EXT_TIMESTAMP] = extPts_;
        cJSON_AddNumberToObject(metaJson, META_EXT_TIMESTAMP.c_str(), extPts_);
    }
    if (extFrameNum_ > 0) {
        // metaJson[META_EXT_FRAME_NUMBER] = extFrameNum_;
        cJSON_AddNumberToObject(metaJson, META_EXT_FRAME_NUMBER.c_str(), extFrameNum_);
    }
    char *jsonstr = cJSON_Print(metaJson);
    if (jsonstr == nullptr) {
        cJSON_Delete(metaJson);
        return "";
    }
    return std::string(metaJson);
}

bool AVTransVideoBufferMeta::UnmarshalVideoMeta(const std::string& jsonStr)
{
    // nlohmann::json metaJson = nlohmann::json::parse(jsonStr, nullptr, false);
    // if (metaJson.is_discarded()) {
    //     return false;
    // }
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr || !cJSON_IsObject(metaJson)) {
        return false;
    }
    if (IsUInt32(metaJson, META_DATA_TYPE)) {
        dataType_ = metaJson[META_DATA_TYPE].get<BufferDataType>();
    }
    if (IsInt64(metaJson, META_TIMESTAMP)) {
        pts_ = metaJson[META_TIMESTAMP].get<int64_t>();
    }
    if (IsUInt32(metaJson, META_FRAME_NUMBER)) {
        frameNum_ = metaJson[META_FRAME_NUMBER].get<uint32_t>();
    }
    if (IsInt64(metaJson, META_EXT_TIMESTAMP)) {
        extPts_ = metaJson[META_EXT_TIMESTAMP].get<int64_t>();
    }
    if (IsUInt32(metaJson, META_EXT_FRAME_NUMBER)) {
        extFrameNum_ = metaJson[META_EXT_FRAME_NUMBER].get<uint32_t>();
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS