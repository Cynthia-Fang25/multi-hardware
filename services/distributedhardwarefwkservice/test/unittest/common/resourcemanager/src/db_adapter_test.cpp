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

#include "db_adapter_test.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "capability_info.h"
#define private public
#include "db_adapter.h"
#undef private
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_db_change_listener.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DbAdapterTest"

namespace {
const string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const string DEV_ID_0 = "bb536a637105409e904d4da83790aa11";
const string DEV_ID_1 = "bb536a637105409e904d4da83790bb22";
const string DEV_NAME = "Dev1";
const string DH_ID_0 = "Camera_00";
const string DH_ID_1 = "Mic_01";
const string DH_ID_2 = "Gps_02";
const string DH_ID_3 = "Display_03";
const string DH_ID_4 = "Input_04";
const string DH_ATTR_0 = "db_test_attr0";
const string DH_ATTR_1 = "db_test_attr1";
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;

const shared_ptr<CapabilityInfo> CAP_INFO_0 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_1 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::AUDIO, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_2 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::GPS, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_3 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SCREEN, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_4 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::INPUT, DH_ATTR_0);

const shared_ptr<CapabilityInfo> CAP_INFO_5 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_6 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::AUDIO, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_7 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::GPS, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_8 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SCREEN, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_9 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::INPUT, DH_ATTR_1);

std::shared_ptr<DBAdapter> g_dbAdapterPtr;
}

void DbAdapterTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }

    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = std::make_shared<MockDBChangeListener>();
    g_dbAdapterPtr = std::make_shared<DBAdapter>(APP_ID, GLOBAL_CAPABILITY_ID, changeListener);
}

void DbAdapterTest::TearDownTestCase(void)
{
    g_dbAdapterPtr->UnInit();

    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }
}

void DbAdapterTest::SetUp()
{
}

void DbAdapterTest::TearDown()
{
}

/**
 * @tc.name: db_adapter_test_000
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(DbAdapterTest, db_adapter_test_000, TestSize.Level0)
{
    EXPECT_EQ(g_dbAdapterPtr->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: db_adapter_test_001
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(DbAdapterTest, db_adapter_test_001, TestSize.Level0)
{
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };

    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::string key;
    for (auto &resInfo : resInfos) {
        key = resInfo->GetKey();
        keys.push_back(key);
        values.push_back(resInfo->ToJsonString());
    }
    EXPECT_EQ(g_dbAdapterPtr->PutDataBatch(keys, values), DH_FWK_SUCCESS);
}

/**
 * @tc.name: db_adapter_test_002
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_002, TestSize.Level0)
{
    std::vector<std::string> keysEmpty;
    std::vector<std::string> valuesEmpty;
    std::vector<std::string> keys { std::string(DEV_ID_0 + DH_ID_0) };
    std::vector<std::string> values { DH_ATTR_0 };

    EXPECT_EQ(g_dbAdapterPtr->PutDataBatch(keys, values), DH_FWK_SUCCESS);
    EXPECT_EQ(g_dbAdapterPtr->PutDataBatch(keys, valuesEmpty), ERR_DH_FWK_PARA_INVALID);
    EXPECT_EQ(g_dbAdapterPtr->PutDataBatch(keysEmpty, values), ERR_DH_FWK_PARA_INVALID);
    EXPECT_EQ(g_dbAdapterPtr->PutDataBatch(keysEmpty, valuesEmpty), ERR_DH_FWK_PARA_INVALID);
}
} // namespace DistributedHardware
} // namespace OHOS
