#include "src/array/service/io_translator/io_translator.h"

#include <gtest/gtest.h>

#include "src/array/ft/raid10.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/array/partition/stripe_partition_mock.h"

using ::testing::Return;
namespace pos
{
TEST(IOTranslator, IOTranslator_testConstructor)
{
    // Given

    // When
    IOTranslator ioTranslator;

    // Then
}

TEST(IOTranslator, Register_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    // When
    bool actual = ioTranslator.Register(mockArrayIndex, trans);

    // Then
    ASSERT_TRUE(actual);
}

TEST(IOTranslator, Unregister_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    PartitionPhysicalSize physicalSize{
        .startLba = 5,
        .lastLba = 100/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    ioTranslator.Register(mockArrayIndex, trans);

    // When
    ioTranslator.Unregister(mockArrayIndex);

    // Then
}

TEST(IOTranslator, Translate_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    ioTranslator.Register(mockArrayIndex, trans);
    int TRANSLATE_SUCCESS = 0;
    EXPECT_CALL(*mockPart, Translate).WillOnce(Return(TRANSLATE_SUCCESS));

    // When
    LogicalEntry src;
    list<PhysicalEntry> dst;
    int actual = ioTranslator.Translate(mockArrayIndex, mockPartitionType, dst, src);

    // Then
    ASSERT_EQ(TRANSLATE_SUCCESS, actual);
}

TEST(IOTranslator, Translate_testIfThereIsNoTranslator)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    int NO_TRANSLATOR = EID(IO_TRANSLATOR_NOT_FOUND);

    // When
    LogicalEntry src;
    list<PhysicalEntry> dst;
    int actual = ioTranslator.Translate(mockArrayIndex, mockPartitionType, dst, src);

    // Then
    ASSERT_EQ(NO_TRANSLATOR, actual);
}

TEST(IOTranslator, GetParityList_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    ioTranslator.Register(mockArrayIndex, trans);
    int SUCCESS = 0;
    EXPECT_CALL(*mockPart, GetParityList).WillOnce(Return(SUCCESS));

    // When
    LogicalWriteEntry src;
    list<PhysicalWriteEntry> dst;
    int actual = ioTranslator.GetParityList(mockArrayIndex, mockPartitionType, dst, src);

    // Then
    ASSERT_EQ(SUCCESS, actual);
}


TEST(IOTranslator, ByteTranslate_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    ioTranslator.Register(mockArrayIndex, trans);
    bool BYTEACCESS_SUPPORTED = true;
    int TRANSLATE_SUCCESS = 0;
    EXPECT_CALL(*mockPart, IsByteAccessSupported).WillOnce(Return(BYTEACCESS_SUPPORTED));
    EXPECT_CALL(*mockPart, ByteTranslate).WillOnce(Return(TRANSLATE_SUCCESS));

    // When
    LogicalByteAddr src;
    PhysicalByteAddr dst;
    int actual = ioTranslator.ByteTranslate(mockArrayIndex, mockPartitionType, dst, src);

    // Then
    ASSERT_EQ(TRANSLATE_SUCCESS, actual);
}

TEST(IOTranslator, ByteConvert_testIfArgumentsAreValid)
{
    // Given
    IOTranslator ioTranslator;
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    string mockArrayName = "mockArray";
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    vector<ArrayDevice*> devs;
    MockStripePartition* mockPart = new MockStripePartition(mockPartitionType, devs, RaidTypeEnum::RAID5);
    trans.emplace(mockPartitionType, mockPart);
    ioTranslator.Register(mockArrayIndex, trans);
    bool BYTEACCESS_SUPPORTED = true;
    int CONVERT_SUCCESS = 0;
    EXPECT_CALL(*mockPart, IsByteAccessSupported).WillOnce(Return(BYTEACCESS_SUPPORTED));
    EXPECT_CALL(*mockPart, ByteConvert).WillOnce(Return(CONVERT_SUCCESS));

    // When
    LogicalByteWriteEntry src;
    list<PhysicalByteWriteEntry> dst;
    int actual = ioTranslator.ByteConvert(mockArrayIndex, mockPartitionType, dst, src);

    // Then
    ASSERT_EQ(CONVERT_SUCCESS, actual);
}

} // namespace pos
