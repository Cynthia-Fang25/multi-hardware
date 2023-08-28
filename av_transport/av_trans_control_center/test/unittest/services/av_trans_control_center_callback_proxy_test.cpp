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

#include "av_sync_manager_test.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void AVTransControlCenterCallbackProxyTest::SetUpTestCase(void)
{
}

void AVTransControlCenterCallbackProxyTest::TearDownTestCase(void)
{
}

void AVTransControlCenterCallbackProxyTest::SetUp()
{
}

void AVTransControlCenterCallbackProxyTest::TearDown()
{
}

/**
 * @tc.name: av_sync_manager_test_001
 * @tc.desc: Verify the EventBus AddHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackProxyTest, event_bus_test_001, TestSize.Level0)
{
    EXPECT_EQ(g_regHandler, nullptr);

    FakeEvent e(*g_sender, *g_obj);
    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);

    EXPECT_NE(g_regHandler, nullptr);
    EXPECT_EQ(g_regHandler->GetSender(), nullptr);
    EXPECT_EQ(g_regHandler->GetHandler(), (void *)g_listener);
}
