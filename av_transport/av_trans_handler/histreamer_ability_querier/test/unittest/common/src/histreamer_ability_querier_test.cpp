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

#include "histreamer_ability_querier_test.h"
#include "histreamer_ability_querier.h"

#include <string>
#include <cstring>
#include <vector>
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void HistreamerAbilityQuerierTest::SetUpTestCase(void) {}

void HistreamerAbilityQuerierTest::TearDownTestCase(void) {}

void HistreamerAbilityQuerierTest::SetUp() {}

void HistreamerAbilityQuerierTest::TearDown() {}

namespace {
int32_t g_maxMessagesLen = 1 * 1024 * 1024;
static const std::uint16_t NUM = 1;
static const std::string KEY = "key";
static const std::string NAME = "name";
static const std::string MIME = "mime";
static const std::string AD_MPEG_VER = "ad_mpeg_ver";
static const std::string AUDIO_AAC_PROFILE = "aac_profile";
static const std::string AUDIO_AAC_STREAM_FORMAT = "aac_stm_fmt";

static const std::vector<std::string> AUDIO_ENCODER_WANT = { "ffmpegAuEnc_aac" };
static const std::vector<std::string> AUDIO_DECODER_WANT = { "ffmpegAuDec_aac" };
static const std::vector<std::string> VIDEO_ENCODER_WANT = {
    "HdiCodecAdapter.OMX.hisi.video.encoder.hevc",
    "HdiCodecAdapter.OMX.hisi.video.encoder.avc",
    "HdiCodecAdapter.OMX.rk.video_encoder.hevc",
    "HdiCodecAdapter.OMX.rk.video_encoder.avc"
    };
static const std::vector<std::string> VIDEO_DECODER_WANT = {
    "HdiCodecAdapter.OMX.hisi.video.decoder.hevc",
    "HdiCodecAdapter.OMX.hisi.video.decoder.avc",
    "HdiCodecAdapter.OMX.rk.video_decoder.hevc",
    "HdiCodecAdapter.OMX.rk.video_decoder.avc"
    };
}

/**
 * @tc.name:histreamer_ability_querier_test_001
 * @tc.desc: Verify the QueryAudioEncoderAbility function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_001, TestSize.Level0)
{
    std::vector<AudioEncoder> audioEncoders = QueryAudioEncoderAbility();
    EXPECT_FALSE(audioEncoders.empty());
    EXPECT_EQ(NUM, audioEncoders.size());
    EXPECT_STREQ(AUDIO_ENCODER_WANT.front().c_str(), audioEncoders.front().name.c_str());
}

/**
 * @tc.name: histreamer_ability_querier_test_002
 * @tc.desc: Verify QueryAudioDecoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_002, TestSize.Level0)
{
    std::vector<AudioDecoder> audioDecoders = QueryAudioDecoderAbility();
    EXPECT_FALSE(audioDecoders.empty());
    EXPECT_EQ(NUM, audioDecoders.size());
    EXPECT_STREQ(AUDIO_DECODER_WANT.front().c_str(), audioDecoders.front().name.c_str());
}

/**
 * @tc.name: histreamer_ability_querier_test_003
 * @tc.desc: Verify QueryVideoEncoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_003, TestSize.Level0)
{
    std::vector<VideoEncoder> videoEncoders = QueryVideoEncoderAbility();
    EXPECT_FALSE(videoEncoders.empty() || videoEncoders.size() > 4);
    for (std::uint16_t i = 0; i < videoEncoders.size(); i++) {
        auto it = find(VIDEO_ENCODER_WANT.begin(), VIDEO_ENCODER_WANT.end(), videoEncoders[i].name);
        EXPECT_TRUE(it != VIDEO_ENCODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_004
 * @tc.desc: Verify QueryVideoDecoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 *
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_004, TestSize.Level0)
{
    std::vector<VideoDecoder> videoDecoders = QueryVideoDecoderAbility();
    EXPECT_FALSE(videoDecoders.empty() || videoDecoders.size() > 4);
    for (std::uint16_t i = 0; i < videoDecoders.size(); i++) {
        auto it = find(VIDEO_DECODER_WANT.begin(), VIDEO_DECODER_WANT.end(), videoDecoders[i].name);
        EXPECT_TRUE(it != VIDEO_DECODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_005
 * @tc.desc: Verify QueryAudioEncoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_005, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryAudioEncoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryAudioEncoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_006
 * @tc.desc: Verify QueryAudioDecoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_006, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryAudioDecoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryAudioDecoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_007
 * @tc.desc: Verify QueryVideoEncoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_007, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryVideoEncoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryVideoEncoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_008
 * @tc.desc: Verify the QueryVideoDecoderAbilityStr function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_008, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryVideoDecoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryVideoDecoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_009
 * @tc.desc: Verify the IsString function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_009, TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject[KEY] = "testing str";
    EXPECT_TRUE(IsString(jsonObject, KEY));

    jsonObject[KEY] = 1;
    EXPECT_FALSE(IsString(jsonObject, KEY));
}

/**
 * @tc.name: histreamer_ability_querier_test_010
 * @tc.desc: Verify the IsUint8 function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_010, TestSize.Level0)
{
    nlohmann::json jsonObject;
    const uint8_t num_s = 1;
    jsonObject[KEY] = num_s;
    EXPECT_TRUE(IsUInt8(jsonObject, KEY));

    const uint16_t num_b = UINT8_MAX + 1 ;
    jsonObject[KEY] = num_b;
    EXPECT_FALSE(IsUInt8(jsonObject, KEY));
}

/**
 * @tc.name: histreamer_ability_querier_test_011
 * @tc.desc: Verify the IsUInt32 function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_011, TestSize.Level0)
{
    nlohmann::json jsonObject;
    const uint32_t num_s = 1;
    jsonObject[KEY] = num_s;
    EXPECT_TRUE(IsUInt32(jsonObject, KEY));

    const int32_t num_i = -1;
    jsonObject[KEY] = num_i;
    EXPECT_FALSE(IsUInt32(jsonObject, KEY));
}

/**
 * @tc.name: histreamer_ability_querier_test_012
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoderIn &audioEncoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_012, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoderIn audioEncoderIn;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, audioEncoderIn);
    EXPECT_TRUE(audioEncoderIn.mime.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_013
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoderOut &audioEncoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_013, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoderOut audioEncoderOut;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_TRUE(audioEncoderOut.mime.empty());

    jsonObject[MIME] = "audio/mp4a-latm";
    jsonObject[AD_MPEG_VER] = "hello";
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ("audio/mp4a-latm", audioEncoderOut.mime);

    jsonObject[AD_MPEG_VER] = (uint32_t)4;
    jsonObject[AUDIO_AAC_PROFILE] = "hello";
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ(4, (uint32_t)audioEncoderOut.ad_mpeg_ver);

    jsonObject[AUDIO_AAC_PROFILE] = 0;
    jsonObject[AUDIO_AAC_STREAM_FORMAT] = "hello";
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ(0, (uint32_t)audioEncoderOut.aac_profile);
}

/**
 * @tc.name: histreamer_ability_querier_test_014
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoder &audioEncoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_014, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoder audioEncoder;
    jsonObject[NAME] = 1;
    FromJson(jsonObject, audioEncoder);
    EXPECT_TRUE(audioEncoder.name.empty());

    jsonObject[NAME] = "ffmpegAuEnc_aac";
    FromJson(jsonObject, audioEncoder);
    EXPECT_EQ("ffmpegAuEnc_aac", audioEncoder.name);
    EXPECT_TRUE(audioEncoder.ins.empty());

    AudioEncoderIn audioEncoderIn;
    audioEncoderIn.mime = "audio/raw";
    audioEncoderIn.sample_rate = {96000, 88200, 64000, 48000, 44100, 32000};
    audioEncoder.ins.push_back(audioEncoderIn);
    FromJson(jsonObject, audioEncoder);
    EXPECT_FALSE(audioEncoder.ins.empty());
    EXPECT_TRUE(audioEncoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_015
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoderIn &audioDecoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_015, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoderIn audioDecoderIn;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, audioDecoderIn);
    EXPECT_TRUE(audioDecoderIn.mime.empty());

    jsonObject[MIME] = "audio/mp4a-latm";
    FromJson(jsonObject, audioDecoderIn);
    EXPECT_EQ("audio/mp4a-latm", audioDecoderIn.mime);
    EXPECT_TRUE(audioDecoderIn.channel_layout.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_016
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoderOut &audioDecoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_016, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoderOut audioDecoderOut;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, audioDecoderOut);
    EXPECT_TRUE(audioDecoderOut.mime.empty());

    jsonObject[MIME] = "audio/raw";
    FromJson(jsonObject, audioDecoderOut);
    EXPECT_EQ("audio/raw", audioDecoderOut.mime);
    EXPECT_TRUE(audioDecoderOut.sample_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_017
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoder &audioDecoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_017, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoder audioDecoder;
    jsonObject[NAME] = 1;
    FromJson(jsonObject, audioDecoder);
    EXPECT_TRUE(audioDecoder.name.empty());

    jsonObject[NAME] = "ffmpegAuDec_aac";
    FromJson(jsonObject, audioDecoder);
    EXPECT_EQ("ffmpegAuDec_aac", audioDecoder.name);
    EXPECT_TRUE(audioDecoder.ins.empty());

    AudioDecoderIn audioDecoderIn;
    audioDecoderIn.mime = "audio/mp4a-latm";
    audioDecoderIn.channel_layout = {
        AudioChannelLayout::CH_2POINT1,
        AudioChannelLayout::CH_2_1,
        AudioChannelLayout::SURROUND,
    };
    audioDecoder.ins.push_back(audioDecoderIn);
    FromJson(jsonObject, audioDecoder);
    EXPECT_FALSE(audioDecoder.ins.empty());
    EXPECT_TRUE(audioDecoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_018
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoderIn &videoEncoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_018, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoderIn videoEncoderIn;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, videoEncoderIn);
    EXPECT_TRUE(videoEncoderIn.mime.empty());

    jsonObject[MIME] = "video/raw";
    FromJson(jsonObject, videoEncoderIn);
    EXPECT_EQ("video/raw", videoEncoderIn.mime);
    EXPECT_TRUE(videoEncoderIn.pixel_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_019
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoderOut &videoEncoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_019, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoderOut videoEncoderOut;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, videoEncoderOut);
    EXPECT_TRUE(videoEncoderOut.mime.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_020
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoder &videoEncoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_020, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoder videoEncoder;
    jsonObject[NAME] = 1;
    FromJson(jsonObject, videoEncoder);
    EXPECT_TRUE(videoEncoder.name.empty());

    jsonObject[NAME] = "HdiCodecAdapter.OMX.rk.video_encoder.hevc";
    FromJson(jsonObject, videoEncoder);
    EXPECT_EQ("HdiCodecAdapter.OMX.rk.video_encoder.hevc", videoEncoder.name);
    EXPECT_TRUE(videoEncoder.ins.empty());

    VideoEncoderIn videoEncoderIn;
    videoEncoderIn.mime = "vide/raw";
    videoEncoderIn.pixel_fmt = {
        VideoPixelFormat::YUV410P,
        VideoPixelFormat::RGBA,
    };
    videoEncoder.ins.push_back(videoEncoderIn);
    FromJson(jsonObject, videoEncoder);
    EXPECT_FALSE(videoEncoder.ins.empty());
    EXPECT_TRUE(videoEncoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_021
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoderIn &videoDecoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_021, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoderIn videoDecoderIn;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, videoDecoderIn);
    EXPECT_TRUE(videoDecoderIn.mime.empty());

    jsonObject[MIME] = "video/hevc";
    FromJson(jsonObject, videoDecoderIn);
    EXPECT_EQ("video/hevc", videoDecoderIn.mime);
    EXPECT_TRUE(videoDecoderIn.vd_bit_stream_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_022
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoderOut &videoDecoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_022, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoderOut videoDecoderOut;
    jsonObject[MIME] = 1;
    FromJson(jsonObject, videoDecoderOut);
    EXPECT_TRUE(videoDecoderOut.mime.empty());

    jsonObject[MIME] = "video/raw";
    FromJson(jsonObject, videoDecoderOut);
    EXPECT_EQ("video/raw", videoDecoderOut.mime);
    EXPECT_TRUE(videoDecoderOut.pixel_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_querier_test_023
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoder &videoDecoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_023, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoder videoDecoder;
    jsonObject[NAME] = 1;
    FromJson(jsonObject, videoDecoder);
    EXPECT_TRUE(videoDecoder.name.empty());

    jsonObject[NAME] = "HdiCodecAdapter.OMX.rk.video_decoder.hevc";
    FromJson(jsonObject, videoDecoder);
    EXPECT_EQ("HdiCodecAdapter.OMX.rk.video_decoder.hevc", videoDecoder.name);
    EXPECT_TRUE(videoDecoder.ins.empty());

    VideoDecoderIn videoDecoderIn;
    videoDecoderIn.mime = "vide/avc";
    videoDecoderIn.vd_bit_stream_fmt = {
        VideoBitStreamFormat::AVC1,
        VideoBitStreamFormat::HEVC,
    };
    videoDecoder.ins.push_back(videoDecoderIn);
    FromJson(jsonObject, videoDecoder);
    EXPECT_FALSE(videoDecoder.ins.empty());
    EXPECT_TRUE(videoDecoder.outs.empty());
}

} // namespace DistributedHardware
} // namespace OHOS