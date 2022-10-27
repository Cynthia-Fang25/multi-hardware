/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
}

class AccessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AccessManagerTest::SetUp()
{
}

void AccessManagerTest::TearDown()
{
}

void AccessManagerTest::SetUpTestCase() {}

void AccessManagerTest::TearDownTestCase() {}

/**
 * @tc.name: AccessManagerInit
 * @tc.desc: Verify the  Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, AccessManagerInit, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceReady
 * @tc.desc: Verify the  OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
    EXPECT_EQ(DistributedHardwareManagerFactory::GetInstance().IsInit(), false);
}

/**
 * @tc.name: OnDeviceOffline
 * @tc.desc: Verify the  OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
    EXPECT_EQ(DistributedHardwareManagerFactory::GetInstance().IsInit(), false);
}
} // namespace DistributedHardware
} // namespace OHOS
