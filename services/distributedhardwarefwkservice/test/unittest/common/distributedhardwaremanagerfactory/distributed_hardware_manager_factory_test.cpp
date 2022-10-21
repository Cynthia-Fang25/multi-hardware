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
enum class Status : uint32_t {
    DEVICE_ONLINE = 0,
    DEVICE_OFFLINE = 1,
};

constexpr int32_t INTERVAL_TIME_MS = 100;
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;

/* save networkId and uuid */
const std::vector<std::pair<std::string, std::string>> TEST_DEVICES = {
    { "11111111111111111111111111111111", "22222222222222222222222222222222" },
    { "33333333333333333333333333333333", "44444444444444444444444444444444" },
    { "55555555555555555555555555555555", "66666666666666666666666666666666" },
    { "77777777777777777777777777777777", "88888888888888888888888888888888" },
};
}

class DistributedHardwareManagerFactoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::mutex testAccessMutex_;
};

void DistributedHardwareManagerFactoryTest::SetUp()
{
    // at last one device online, ensure sa not exit
    std::string networkId = "00000000000000000000000000000000";
    std::string uuid = "99999999999999999999999999999999";
    DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, TEST_DEV_TYPE_PAD);
}

void DistributedHardwareManagerFactoryTest::TearDown()
{
    // clear all the online devices
    for (const auto &dev : TEST_DEVICES) {
        DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
    }
}

void DistributedHardwareManagerFactoryTest::SetUpTestCase() {}

void DistributedHardwareManagerFactoryTest::TearDownTestCase() {}

/**
 * @tc.name: SendOnLineEvent_001
 * @tc.desc: Verify the online success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOnLineEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_EQ(DH_FWK_SUCCESS, ret);
        ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
    }
}

/**
 * @tc.name: SendOnLineEvent_002
 * @tc.desc: Verify the online failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOnLineEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    auto ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent("", TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first, "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_001
 * @tc.desc: Verify the offline success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_EQ(DH_FWK_SUCCESS, ret);
        ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
    }

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_EQ(DH_FWK_SUCCESS, ret);
    }
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_002
 * @tc.desc: Verify the offline failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent("", TEST_DEVICES[0].second,
        TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    ret =
        DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first, "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_003
 * @tc.desc: Verify the  SendOnLineEvent for Multi-thread
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_003, TestSize.Level0)
{
    auto handler = [this](Status status, std::string networkId, std::string uuid, int32_t expect) {
        if (status == Status::DEVICE_ONLINE) {
            std::lock_guard<std::mutex> lock(testAccessMutex_);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto onlineResult =
                DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, TEST_DEV_TYPE_PAD);
            EXPECT_EQ(expect, onlineResult);
        } else {
            std::lock_guard<std::mutex> lock(testAccessMutex_);
            std::this_thread::sleep_for(std::chrono::milliseconds(90));
            auto offlineResult =
                DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId, uuid, TEST_DEV_TYPE_PAD);
            EXPECT_EQ(expect, offlineResult);
        }
    };

    std::vector<std::thread> threadVec;
    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_ONLINE, TEST_DEVICES[0].first, TEST_DEVICES[0].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_ONLINE, TEST_DEVICES[1].first, TEST_DEVICES[1].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(std::thread(handler, Status::DEVICE_ONLINE, TEST_DEVICES[0].first, TEST_DEVICES[0].second,
        ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_ONLINE, TEST_DEVICES[2].first, TEST_DEVICES[2].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_ONLINE, TEST_DEVICES[3].first, TEST_DEVICES[3].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_OFFLINE, TEST_DEVICES[3].first, TEST_DEVICES[3].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(std::thread(handler, Status::DEVICE_OFFLINE, TEST_DEVICES[3].first, TEST_DEVICES[3].second,
        ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_OFFLINE, TEST_DEVICES[1].first, TEST_DEVICES[1].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_OFFLINE, TEST_DEVICES[0].first, TEST_DEVICES[0].second, DH_FWK_SUCCESS));
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_TIME_MS));

    threadVec.emplace_back(
        std::thread(handler, Status::DEVICE_OFFLINE, TEST_DEVICES[2].first, TEST_DEVICES[2].second, DH_FWK_SUCCESS));

    for_each(threadVec.begin(), threadVec.end(), [](std::thread &t) { t.join(); });
}

/**
 * @tc.name: SendOffLineEvent_004
 * @tc.desc: Verify the  SendOnLineEvent
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_004, TestSize.Level0)
{
    std::string networkIdEmpty = "";
    std::string uuid = "uu1234567890";
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(
        networkIdEmpty, uuid, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: SendOffLineEvent_005
 * @tc.desc: Verify the  SendOnLineEvent
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_005, TestSize.Level0)
{
    std::string networkId = "nt12345678790";
    std::string uuidEmpty = "";
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(
        networkId, uuidEmpty, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: SendOffLineEvent_006
 * @tc.desc: Verify the  SendOffLineEvent
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_006, TestSize.Level0)
{
    std::string networkIdEmpty = "";
    std::string uuid = "uu1234567890";
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(
        networkIdEmpty, uuid, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: SendOffLineEvent_007
 * @tc.desc: Verify the  SendOffLineEvent
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerFactoryTest, SendOffLineEvent_007, TestSize.Level0)
{
    std::string networkId = "nt12345678790";
    std::string uuidEmpty = "";
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(
        networkId, uuid, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
