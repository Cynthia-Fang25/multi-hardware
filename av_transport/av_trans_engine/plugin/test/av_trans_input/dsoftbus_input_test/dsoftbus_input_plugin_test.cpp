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

#include "dsoftbus_input_plugin_test.h"

#include "dsoftbus_input_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dsoftbus_input_plugin";

void DsoftbusInputPluginTest::SetUpTestCase(void) {}

void DsoftbusInputPluginTest::TearDownTestCase(void) {}

void DsoftbusInputPluginTest::SetUp(void) {}

void DsoftbusInputPluginTest::TearDown(void) {}

class DaudioInputPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

HWTEST_F(DsoftbusInputPluginTest, Prepare_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->peerDevId_ = "peerDevId_";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Stop_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::RUNNING;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    std::string value = "dsoftbus_input_test";
    Status ret = plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    EXPECT_EQ(Status::OK, ret);

    bool val = true;
    ret = plugin->SetParameter(Tag::SECTION_USER_SPECIFIC_START, val);
    EXPECT_EQ(Status::OK, ret);

    ret = plugin->SetParameter(Tag::SECTION_VIDEO_SPECIFIC_START, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_input_test";
    plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->paramsMap_.clear();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);

    AVTransEvent event;
    plugin->OnChannelEvent(event);

    plugin->SetDumpFlagFalse();
    std::shared_ptr<AVBuffer> buffer = std::make_shared<AVBuffer>();
    plugin->DataEnqueue(buffer);
}

HWTEST_F(DsoftbusInputPluginTest, Pause_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    Callback *cb = new DaudioInputPluginCallback();
    ret = plugin->SetCallback(cb);
    EXPECT_EQ(Status::OK, ret);
    delete cb;
}

HWTEST_F(DsoftbusInputPluginTest, Resume_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Resume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetDataCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    AVDataCallback dataCb;
    Status ret = plugin->SetDataCallback(dataCb);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Deinit_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    Status ret = plugin->Deinit();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Reset_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_OPENED;
    plugin->eventsCb_ = new DaudioInputPluginCallback();
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_CHANNEL_CLOSED;
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_START_SUCCESS;
    plugin->OnChannelEvent(event);
    delete plugin->eventsCb_;
    AVTRANS_LOGI("zlf 2");
    plugin->isrunning_ = false;
    plugin->HandleData();
}
} // namespace DistributedHardware
} // namespace OHOS