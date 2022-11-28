/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <securec.h>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "dm_device_info.h"
#define private public
#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"
#undef private
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

class AccessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::mutex testAccessMutex_;
};

void AccessManagerTest::SetUp()
{
    // at last one device online, ensure sa not exit
    std::string networkId = "00000000000000000000000000000000";
    std::string uuid = "99999999999999999999999999999999";
    DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, TEST_DEV_TYPE_PAD);
}

void AccessManagerTest::TearDown()
{
    // clear all the online devices
    for (const auto &dev : TEST_DEVICES) {
        DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
    }
}

void AccessManagerTest::SetUpTestCase() {}

void AccessManagerTest::TearDownTestCase() {}

/**
 * @tc.name: SendOnLineEvent_001
 * @tc.desc: Verify the online success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AccessManagerTest, SendOnLineEvent_001, TestSize.Level1)
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
HWTEST_F(AccessManagerTest, SendOnLineEvent_002, TestSize.Level1)
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
HWTEST_F(AccessManagerTest, SendOffLineEvent_001, TestSize.Level1)
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
HWTEST_F(AccessManagerTest, SendOffLineEvent_002, TestSize.Level1)
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
HWTEST_F(AccessManagerTest, SendOffLineEvent_003, TestSize.Level0)
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
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Verify the  OnRemoteDied function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnRemoteDied_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    AccessManager::GetInstance()->OnRemoteDied();
}

/**
 * @tc.name: OnDeviceOnline_001
 * @tc.desc: Verify the  OnDeviceOnline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOnline_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceOnline(deviceInfo);
}

/**
 * @tc.name: OnDeviceOffline_001
 * @tc.desc: Verify the  OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
}

/**
 * @tc.name: OnDeviceOffline_002
 * @tc.desc: Verify the  OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_002, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length()));
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
}

/**
 * @tc.name: OnDeviceOffline_003
 * @tc.desc: Verify the  OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_003, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length()));
    std::string netId = "000002";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, netId.c_str(), netId.length()));
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
}

/**
 * @tc.name: OnDeviceReady_001
 * @tc.desc: Verify the  OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
}

/**
 * @tc.name: OnDeviceReady_002
 * @tc.desc: Verify the  OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_002, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length()));
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
}

/**
 * @tc.name: OnDeviceReady_003
 * @tc.desc: Verify the  OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_003, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length()));
    std::string netId = "000002";
    EXPECT_EQ(EOK, memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, netId.c_str(), netId.length()));
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
}

/**
 * @tc.name: OnDeviceChanged_001
 * @tc.desc: Verify the  OnDeviceChanged function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceChanged_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceChanged(deviceInfo);
}

/**
 * @tc.name: UnInit_001
 * @tc.desc: Verify the  UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, UnInit_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceChanged(deviceInfo);
}
} // namespace DistributedHardware
} // namespace OHOS
