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

#include "daudio_output_test.h"

#include "daudio_output_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "daudio_output";

void DaudioOutputTest::SetUpTestCase() {}

void DaudioOutputTest::TearDownTestCase() {}

void DaudioOutputTest::SetUp() {}

void DaudioOutputTest::TearDown() {}

HWTEST_F(DaudioOutputTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DaudioOutputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_test";
    Status ret = plugin->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_test";
    plugin->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_AV_SYNC_GROUP_INFO, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->paramsMap_.clear();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_AV_SYNC_GROUP_INFO, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DaudioOutputTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DaudioOutputTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->sendPlayTask_ = std::make_shared<Media::OSAL::Task>("sendPlayTask_");
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DaudioOutputTest, Stop_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->sendPlayTask_ = std::make_shared<Media::OSAL::Task>("sendPlayTask_");
    plugin->state_ = State::RUNNING;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

} // namespace DistributedHardware
} // namespace OHOS