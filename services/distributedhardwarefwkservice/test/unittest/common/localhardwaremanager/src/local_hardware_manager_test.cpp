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

#include "local_hardware_manager_test.h"

#define private public
#include "capability_info_manager.h"
#include "component_loader.h"
#include "local_hardware_manager.h"
#undef private

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    std::map<DHType, CompHandler> g_compHandlerMap;
}

void LocalHardwareManagerTest::SetUpTestCase(void)
{
    ComponentLoader::GetInstance().Init();
}

void LocalHardwareManagerTest::TearDownTestCase(void)
{
    ComponentLoader::GetInstance().UnInit();
}

void LocalHardwareManagerTest::SetUp() {}

void LocalHardwareManagerTest::TearDown() {}

/**
 * @tc.name: local_hardware_manager_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, local_hardware_manager_test_001, TestSize.Level0)
{
    LocalHardwareManager::GetInstance().Init();
}

/**
 * @tc.name: local_hardware_manager_test_002
 * @tc.desc: Verify the UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, local_hardware_manager_test_002, TestSize.Level0)
{
    LocalHardwareManager::GetInstance().UnInit();
}

} // namespace DistributedHardware
} // namespace OHOS
