#include "src/io/backend_io/stripe_map_update.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/wb_stripe_manager/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_completion.h"
#include "src/journal_service/journal_service.h"
#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_service/i_journal_writer_mock.h"
#include "test/unit-tests/journal_service/journal_service_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(StripeMapUpdate, StripeMapUpdate_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name{"arr_name"};

    // When: Try to Create New StripeMapUpdate object with 1 argument
    StripeMapUpdate stripeMapUpdate(&mockStripe, arr_name);

    // Then: Do nothing
}

TEST(StripeMapUpdate, StripeMapUpdate_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name{"arr_name"};

    // When: Try to Create New StripeMapUpdate object with 1 argument
    StripeMapUpdate* stripeMapUpdate = new StripeMapUpdate(&mockStripe, arr_name);

    // Then: Release memory
    delete stripeMapUpdate;
}

TEST(StripeMapUpdate, StripeMapUpdate_Constructor_FourArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};

    // When: Try to Create New StripeMapUpdate object with 4 argument
    StripeMapUpdate stripeMapUpdate(&mockStripe, &mockIStripeMap, &mockJournalService, &mockEventScheduler, arr_name);

    // Then: Do nothing
}

TEST(StripeMapUpdate, StripeMapUpdate_Execute_NormalWithJournalOn)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockIJournalWriter> mockIJournalWriter;
    NiceMock<MockIStripeMap> mockIStripeMap;
    std::string arr_name{"arr_name"};

    StripeMapUpdate stripeMapUpdate(&mockStripe, &mockIStripeMap, &mockJournalService, &mockEventScheduler, arr_name);

    MpageList dirty;
    StripeAddr stripeAddr;
    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(true));
    ON_CALL(mockJournalService, GetWriter(_)).WillByDefault(Return(&mockIJournalWriter));
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetDirtyStripeMapPages(_)).WillByDefault(Return(dirty));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIJournalWriter, AddStripeMapUpdatedLog(_, _, _, _)).WillByDefault(Return(0));

    bool actual, expected{true};

    // When: Journal is enabled, journal->AddStripeMapUpdatedLog(_) == 0
    actual = stripeMapUpdate.Execute();

    // Then: Return true;
    ASSERT_EQ(expected, actual);
}

TEST(StripeMapUpdate, StripeMapUpdate_Execute_AddStripeMapUpdatedLogFailWithJournalOn)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockIJournalWriter> mockIJournalWriter;
    std::string arr_name{"arr_name"};

    StripeMapUpdate stripeMapUpdate(&mockStripe, &mockIStripeMap, &mockJournalService, &mockEventScheduler, arr_name);
    MpageList dirty;
    StripeAddr stripeAddr;
    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(true));
    ON_CALL(mockJournalService, GetWriter(_)).WillByDefault(Return(&mockIJournalWriter));
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetDirtyStripeMapPages(_)).WillByDefault(Return(dirty));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIJournalWriter, AddStripeMapUpdatedLog(_, _, _, _)).WillByDefault(Return(-1));

    bool actual, expected{false};

    // When: Journal is enabled, journal->AddStripeMapUpdatedLog(_) != 0
    actual = stripeMapUpdate.Execute();

    // Then: Return false;
    ASSERT_EQ(expected, actual);
}

} // namespace pos
