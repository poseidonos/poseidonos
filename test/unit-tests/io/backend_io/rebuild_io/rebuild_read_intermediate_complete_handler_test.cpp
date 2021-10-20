#include "src/io/backend_io/rebuild_io/rebuild_read_intermediate_complete_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/ubio_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(RebuildReadIntermediateCompleteHandler, RebuildReadIntermediateCompleteHandler_Heap)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadIntermediateCompleteHandler object on heap
    RebuildReadIntermediateCompleteHandler* rebuildReadIntermediateCompleteHandler = new RebuildReadIntermediateCompleteHandler(ubio);

    // Then : Delete resource
    delete rebuildReadIntermediateCompleteHandler;
}

TEST(RebuildReadIntermediateCompleteHandler, RebuildReadIntermediateCompleteHandler_Stack)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadIntermediateCompleteHandler object on stack
    RebuildReadIntermediateCompleteHandler rebuildReadIntermediateCompleteHandler(ubio);

    // Then : Do nothing
}

TEST(RebuildReadIntermediateCompleteHandler, RebuildReadIntermediateCompleteHandler_DoSpecificJob)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadIntermediateCompleteHandler
    RebuildReadIntermediateCompleteHandler rebuildReadIntermediateCompleteHandler(ubio);
    rebuildReadIntermediateCompleteHandler.Execute();

    // Then : Do nothing
}

TEST(RebuildReadIntermediateCompleteHandler, RebuildReadIntermediateCompleteHandler_DoSpecificJob_NullUbio)
{
    // Given

    // When : Create RebuildReadIntermediateCompleteHandler with nullptr ubio
    RebuildReadIntermediateCompleteHandler rebuildReadIntermediateCompleteHandler(nullptr);
    rebuildReadIntermediateCompleteHandler.Execute();

    // Then : Do nothing
}

TEST(RebuildReadIntermediateCompleteHandler, RebuildReadIntermediateCompleteHandler_DoSpecificJob_WithError)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadIntermediateCompleteHandler and set error
    RebuildReadIntermediateCompleteHandler rebuildReadIntermediateCompleteHandler(ubio);
    rebuildReadIntermediateCompleteHandler.InformError(IOErrorType::DEVICE_ERROR);
    rebuildReadIntermediateCompleteHandler.Execute();

    // Then : Do nothing
}

} // namespace pos
