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

#include "resource_manager_test.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "version_info.h"
#define private public
#include "version_info_manager.h"
#undef private
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionInfoManagerTest"

namespace {
const string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const string DEV_ID_1 = "bb536a637105409e904d4da83790a4a7";
const string DEV_ID_2 = "bb536a637105409e904d4da83790a4a8";
const string DEV_ID_3 = "bb536a637105409e904d4da83790a4a9";
const string NAME_SCREEN = "distributed_screen";
const string NAME_CAMERA = "distributed_camera";
const string VERSION_1 = "1.0";
const string VERSION_2 = "2.0";
}

void VersionInfoManagerTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }
}

void VersionInfoManagerTest::TearDownTestCase(void)
{
    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }
}

void VersionInfoManagerTest::SetUp()
{
    VersionInfo verInfo1;
    verInfo1.deviceId = DEV_ID_1;
    CompVersion compVersions;
    compVersions.dhType = DHType::CAMERA;
    compVersions.name = NAME_CAMERA;
    compVersions.handlerVersion = VERSION_1;
    compVersions.sinkVersion = VERSION_1;
    compVersions.sourceVersion = VERSION_1;
    VersionInfoManager::GetInstance()->AddVersion(verInfo1);

    VersionInfo verInfo2;
    verInfo2.deviceId = DEV_ID_2;
    compVersions.dhType = DHType::DISPLAY
    compVersions.name = NAME_CAMERA;
    compVersions.handlerVersion = VERSION_2;
    compVersions.sinkVersion = VERSION_2;
    compVersions.sourceVersion = VERSION_2;
    VersionInfoManager::GetInstance()->AddVersion(verInfo2);
}

void VersionInfoManagerTest::TearDown()
{
    VersionInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_0->GetKey());
}

/**
 * @tc.name:version_info_manager_test_001
 * @tc.desc: Verify the VersionInfoManager UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_001, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
    EXPECT_EQ(VersionInfoManager::GetInstance()->UnInit(), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_002
 * @tc.desc: Verify the VersionInfoManager Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_002, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_003
 * @tc.desc: Verify the VersionInfoManager SyncVersionInfoFromDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_003, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(DEV_ID_1), DH_FWK_SUCCESS);
    EXPECT_EQ(VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(DEV_ID_2), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_004
 * @tc.desc: Verify the VersionInfoManager SyncRemoteVersionInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_004, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->SyncRemoteVersionInfos(), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_005
 * @tc.desc: Verify the VersionInfoManager AddVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_005, TestSize.Level0)
{
    VersionInfo verInfo;
    verInfo.deviceId = DEV_ID_3;
    CompVersion compVersions;
    compVersions.dhType = DHType::CAMERA;
    compVersions.name = NAME_CAMERA;
    compVersions.handlerVersion = VERSION_1;
    compVersions.sinkVersion = VERSION_1;
    compVersions.sourceVersion = VERSION_1;
    EXPECT_EQ(VersionInfoManager::GetInstance()->AddVersion(verInfo), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_006
 * @tc.desc: Verify the VersionInfoManager GetVersionInfoByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_006, TestSize.Level0)
{
    VersionInfo versionInfo;
    EXPECT_EQ(VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(DEV_ID_1, versionInfo), DH_FWK_SUCCESS);
}

} // namespace DistributedHardware
} // namespace OHOS
