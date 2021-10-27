#include "src/mapper_service/mapper_service.h"

#include <gtest/gtest.h>

#include "test/unit-tests/mapper/i_map_flush_mock.h"
#include "test/unit-tests/mapper/i_mapper_wbt_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;

namespace pos
{
TEST(MapperService, UnregisterMapper_TestFail)
{
    MapperService ms;
    ms.UnregisterMapper("aaa");
    ms.GetIVSAMap("bbb");
    ms.GetIStripeMap("ccc");
    ms.GetIReverseMap("ddd");
    NiceMock<MockIMapFlush>* mapflush = new NiceMock<MockIMapFlush>();
    NiceMock<MockIMapperWbt>* wbt = new NiceMock<MockIMapperWbt>();
    ms.RegisterMapper("0", 0, nullptr, nullptr, nullptr, mapflush, wbt);
    ms.GetIMapFlush("0");
    ms.GetIMapFlush("fff");
    ms.GetIMapperWbt("c");
    ms.GetIMapperWbt(0);
}

} // namespace pos
