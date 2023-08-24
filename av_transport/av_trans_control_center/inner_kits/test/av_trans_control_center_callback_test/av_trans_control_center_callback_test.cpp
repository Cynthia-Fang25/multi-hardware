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

#include "av_trans_control_center_callback_test.h"
#include "av_trans_control_center_callback.h"

#include "av_trans_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string TRANSRNAME = "avtrans";

void AvTransControlCenterCallbackTest::SetUp()
{
}

void AvTransControlCenterCallbackTest::TearDown()
{
}

void AvTransControlCenterCallbackTest::SetUpTestCase()
{
}

void AvTransControlCenterCallbackTest::TearDownTestCase()
{
}

HWTEST_F(AvTransControlCenterCallbackTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVTransControlCenterCallback> avTransTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVTransControlCenterCallback>(AVINPUT_NAME, TRANSRNAME);
    AVTransTag tag = AVTransTag::START_AV_SYNC;
    std::string value = VideoBitStreamFormat::ANNEXB;
    int32_t ret = avTransTest_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvTransControlCenterCallbackTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVTransControlCenterCallback> avTransTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVTransControlCenterCallback>(AVINPUT_NAME, TRANSRNAME);
    AVTransTag tag = AVTransTag::STOP_AV_SYNC;
    std::string value = VideoBitStreamFormat::ANNEXB;
    int32_t ret = avTransTest_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvTransControlCenterCallbackTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVTransControlCenterCallback> avTransTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVTransControlCenterCallback>(AVINPUT_NAME, TRANSRNAME);
    AVTransTag tag = AVTransTag::TIME_SYNC_RESULT;
    std::string value = VideoBitStreamFormat::ANNEXB;
    int32_t ret = avTransTest_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvTransControlCenterCallbackTest, SetSharedMemory_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVTransControlCenterCallback> avTransTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVTransControlCenterCallback>(AVINPUT_NAME, TRANSRNAME);
    AVTransSharedMemory memory = sendEngine->SetParameter(AVTransTag::SHARED_MEMORY_FD, MarshalSharedMemory(memory));
    int32_t ret = avTransTest_->SetSharedMemory(memory);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvTransControlCenterCallbackTest, SetSharedMemory_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVTransControlCenterCallback> avTransTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVTransControlCenterCallback>(AVINPUT_NAME, TRANSRNAME);
    AVTransSharedMemory memory = rcvEngine->SetParameter(AVTransTag::SHARED_MEMORY_FD, MarshalSharedMemory(memory));
    int32_t ret = avTransTest_->SetSharedMemory(memory);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    std::shared_ptr<IAVSenderEngine> sender = nullptr;
    avTransTest_->SetSenderEngine(sender);
    std::make_shared<IAVReceiverEngine> sender != nullptr;
    avTransTest_->SetSenderEngine(sender);
    std::shared_ptr<IAVReceiverEngine> receiver = nullptr;
    avTransTest_->SetReceiverEngine(receiver);
    std::make_shared<IAVReceiverEngine> receiver != nullptr;
    avTransTest_->SetReceiverEngine(receiver);
}

} // namespace DistributedHardware
} // namespace OHOS