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


#include <iostream>
#include "av_receiver_engine_test.h"

#include "pipeline/factory/filter_factory.h"
#include "plugin_video_tags.h"
#include "distributed_hardware_fwk_kit.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avreceivererengine";

void AvReceiverEngineTest::SetUp()
{
    AutoRegisterFilter<AVOutputFilter> registerOutputFilter(AVOUTPUT_NAME);
    AutoRegisterFilter<AVInputFilter> registerInputFilter(AVINPUT_NAME);
    AutoRegisterFilter<OHOS::Media::Pipeline::VideoEncoderFilter> registerEncoderFilter(VENCODER_NAME);
}

void AvReceiverEngineTest::TearDown()
{
}

void AvReceiverEngineTest::SetUpTestCase()
{
}

void AvReceiverEngineTest::TearDownTestCase()
{
}

HWTEST_F(AvReceiverEngineTest, Initialize_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->initialized_ = true;
    int32_t ret = receiver->Initialize();
    std::string value264 = MIME_VIDEO_H264;
    receiver->videoEncoder_ =
        FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(VENCODER_NAME, "videoencoder");
    receiver->avInput_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->SetVideoCodecType(value264);
    std::string value = MIME_VIDEO_H265;
    receiver->SetVideoCodecType(value);    
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, Initialize_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Initialize_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->sessionName_ = "";
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, InitPipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->pipeline_ = nullptr;
    receiver->SetEngineReady(ownerName);
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    receiver->SetEngineReady(ownerName);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

HWTEST_F(AvReceiverEngineTest, InitPipeline_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(OWNER_NAME_D_MIC, peerDevId);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds;
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Start_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::IDLE;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->Start();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Start_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->Start();
    receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    receiver->Start();
    int32_t ret = receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::STARTED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVOutputFilter> avOutput_ = nullptr;
    avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    receiver->NotifyStreamChange(type);
    int32_t ret = receiver->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, SendMessage_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SPEAKER;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    receiver->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = receiver->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, RegisterReceiverCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SCREEN;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    receiver->NotifyStreamChange(type);
    std::shared_ptr<ReceiverEngineCallback> callback = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, RegisterReceiverCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    std::shared_ptr<ReceiverEngineCallback> callback = nullptr;
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->receiverCallback_ = nullptr;
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPENED;
    receiver->OnChannelEvent(transEvent);
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::CH_CREATED, receiver->currentState_);
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    AVTransEvent event;
    event.content = "content";
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->OnChannelEvent(event);

    event.type = EventType::EVENT_DATA_RECEIVED;
    receiver->OnChannelEvent(event);

    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
}
} // namespace DistributedHardware
} // namespace OHOS