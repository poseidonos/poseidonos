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
    // When
    AioSubmissionAdapter* aioSubmissionAdapter = new AioSubmissionAdapter;
    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Read;
    try
    {
        aioSubmissionAdapter->Do(volIo);
    }
    catch (...)
    {
    }

    // Then
    delete aioSubmissionAdapter;
}

} // namespace pos
