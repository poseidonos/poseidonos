#include "src/io/frontend_io/aio_submission_adapter.h"

#include <gtest/gtest.h>

#include "src/io/frontend_io/aio.h"

namespace pos
{
TEST(AioSubmissionAdapter, Constructor_Stack)
{
    // Given

    // When
    AioSubmissionAdapter aioSubmissionAdapter;

    // Then
}

TEST(AioSubmissionAdapter, Constructor_Heap)
{
    // Given

    // When
    AioSubmissionAdapter* aioSubmissionAdapter = new AioSubmissionAdapter();

    // Then
    delete aioSubmissionAdapter;
}

TEST(AioSubmissionAdapter, Do_SubmitAsyncIO)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    posIo.ioType = IO_TYPE::FLUSH;

    // When
    AioSubmissionAdapter* aioSubmissionAdapter = new AioSubmissionAdapter();
    aioSubmissionAdapter->Do(&posIo);

    // Then
    delete aioSubmissionAdapter;
}

} // namespace pos
