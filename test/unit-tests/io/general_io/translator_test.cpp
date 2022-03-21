#include "src/io/general_io/translator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/array/service/io_translator/i_io_translator_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/array_mgmt/interface/i_array_mgmt_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class TranslatorTestFixture : public ::testing::Test
{
public:
    TranslatorTestFixture(void)
    {
    }

    virtual ~TranslatorTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        mockIVSAMap = new NiceMock<MockIVSAMap>;
        ON_CALL(*mockIVSAMap, GetVSAs(_, _, _, _)).WillByDefault([](int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray) {
            if (startRba <= 1)
            {
                vsaArray[0] = {.stripeId = 0, .offset = 0};
                vsaArray[1] = {.stripeId = 0, .offset = 1};
            }
            else
            {
                vsaArray[0] = UNMAP_VSA;
            }
            return 0;
        });
        ON_CALL(*mockIVSAMap, GetRandomVSA(_)).WillByDefault(Return(vsa));

        mockIStripeMap = new NiceMock<MockIStripeMap>;
        ON_CALL(*mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
        ON_CALL(*mockIStripeMap, IsInUserDataArea(_)).WillByDefault(Return(true));

        mockWBAllocator = new NiceMock<MockIWBStripeAllocator>;

        mockITranslator = new NiceMock<MockIIOTranslator>;
        PhysicalBlkAddr tmpPba = {.lba = 0, .arrayDev = nullptr};
        PhysicalEntry tmpPE = {.addr = tmpPba,.blkCnt = 1};
        list<PhysicalEntry> tmpPWEs;
        tmpPWEs.push_back(tmpPE);
        ON_CALL(*mockITranslator, Translate(arrayId, _, _, _)).WillByDefault([tmpPWEs](unsigned int arrayIndex, PartitionType part, list<PhysicalEntry>& dst, const LogicalEntry& src)
        {
            dst = tmpPWEs;
            return 0;
        });
        mockIArrayInfo = new NiceMock<MockIArrayInfo>();
    }

    virtual void
    TearDown(void)
    {
        delete mockIVSAMap;
        delete mockIStripeMap;
        delete mockWBAllocator;
        delete mockITranslator;
        delete mockIArrayInfo;
    }

protected:
    std::string arrayName = "POSArray1";
    int arrayId = 0;
    VirtualBlkAddr vsa{.stripeId = 0, .offset = 0};
    StripeAddr stripeAddr;
    NiceMock<MockIVSAMap>* mockIVSAMap;
    NiceMock<MockIStripeMap>* mockIStripeMap;
    NiceMock<MockIWBStripeAllocator>* mockWBAllocator;
    NiceMock<MockIIOTranslator>* mockITranslator;
    NiceMock<MockIArrayInfo>* mockIArrayInfo;
};

TEST_F(TranslatorTestFixture, Translator_Constructor_EightArguments)
{
    //When: create translator for single block (stack)
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);

    //Then: do nothing

    //When: create translator for mulitple blocks
    Translator translator2(0, 0, 2, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    //Then: do nothing

    //When: create translator (heap)
    Translator* pTranslator = new Translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    delete pTranslator;
    //Then: do nothing
}

TEST_F(TranslatorTestFixture, Translator_Constructor_TwoArguments)
{
    //When: create translator (stack)
    Translator translator(vsa, 0, UNMAP_STRIPE, mockIArrayInfo);

    //Then: do nothing

    //Given: register IStripeMap
    MapperServiceSingleton::Instance()->RegisterMapper(arrayName, arrayId, nullptr, mockIStripeMap, nullptr, nullptr, nullptr);

    //When: create translator (heap)
    Translator* pTranslator = new Translator(vsa, 0, UNMAP_STRIPE, mockIArrayInfo);
    delete pTranslator;

    //Then: do nothing

    //Clean up
    MapperServiceSingleton::Instance()->UnregisterMapper(arrayName);
}

TEST_F(TranslatorTestFixture, Translator_Constructor_InvalidVolumeIdException)
{
    //When: create translator with invalid volumeId
    //Then: exception is thrown
    EXPECT_THROW(Translator translator(MAX_VOLUME_COUNT, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo), POS_EVENT_ID);
}

TEST_F(TranslatorTestFixture, GetVsa)
{
    //When: translate to a mapped VSA
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    VirtualBlkAddr actual = translator.GetVsa(0);

    //Then: return a valid vsa
    EXPECT_EQ(actual, vsa);

    //When: get a VSA of invalid range
    actual = translator.GetVsa(1);

    //Then: return an unmapped VSA
    EXPECT_EQ(actual, UNMAP_VSA);

    //When: translate to an unmapped VSA
    Translator translator2(0, 2, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    actual = translator2.GetVsa(0);

    //Then: return an unmapped VSA
    EXPECT_EQ(actual, UNMAP_VSA);
}

TEST_F(TranslatorTestFixture, GetLsidEntry)
{
    //When: translate to a mapped VSA and get lsid entry
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);

    StripeAddr actual = translator.GetLsidEntry(0);

    //Then: return a valid lsidEntry
    StripeAddr expected{.stripeLoc = IN_USER_AREA, .stripeId = 0};
    EXPECT_EQ(actual, expected);

    //When: get a lsid entry of invalid range
    actual = translator.GetLsidEntry(1);

    //Then: return an unmapped stripe
    expected.stripeId = UNMAP_STRIPE;
    EXPECT_EQ(actual, expected);
}

TEST_F(TranslatorTestFixture, GetPba)
{
    //When: get a pba of valid range
    ON_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    PhysicalBlkAddr pba = translator.GetPba();
    PhysicalBlkAddr pba0 = translator.GetPba(0);

    //Then: return a valid pba
    EXPECT_EQ(pba.lba, pba0.lba);
    EXPECT_EQ(pba.arrayDev, pba0.arrayDev);

    //When: get a pba of unmapped LSA
    Translator translator2(0, 2, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    pba = translator2.GetPba();

    //Then: return a default pba
    EXPECT_EQ(pba.lba, 0);
    EXPECT_EQ(pba.arrayDev, nullptr);

    //When: get a pba of invalid range
    //Then: exception is thrown
    EXPECT_THROW(translator.GetPba(1), POS_EVENT_ID);

    //Given: make the stripemap return non-user partition type (write_buffer)
    ON_CALL(*mockIStripeMap, IsInUserDataArea(_)).WillByDefault(Return(false));

    //When: get a pba of LSA in non-user partition
    pba = translator.GetPba(0);

    //Then: return a default pba
    EXPECT_EQ(pba.lba, 0);
    EXPECT_EQ(pba.arrayDev, nullptr);

    //Given: make the translate return fail
    ON_CALL(*mockITranslator, Translate(arrayId, _, _, _)).WillByDefault(Return(-1));

    //When: get a pba of valid range
    //Then: exception is thrown
    EXPECT_THROW(translator.GetPba(0), POS_EVENT_ID);
}

TEST_F(TranslatorTestFixture, GetPhysicalEntries)
{
    //When: get physical entries
    ON_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);
    list<PhysicalEntry> actual = translator.GetPhysicalEntries(nullptr, 0);

    //Then: return a phyiscal entry
    EXPECT_EQ(actual.size(), 1);

    //Given: make the conversion return fail
    ON_CALL(*mockITranslator, Translate(arrayId, _, _, _)).WillByDefault(Return(-1));

    //When: get physical entries
    //Then: exception is thrown
    EXPECT_THROW(translator.GetPhysicalEntries(nullptr, 0), POS_EVENT_ID);
}

TEST_F(TranslatorTestFixture, IsMapped)
{
    //When: translate to a mapped VSA
    Translator translator(0, 0, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);

    //Then: IsMapped returns true
    EXPECT_TRUE(translator.IsMapped());

    //When: translate to an unmapped VSA
    Translator translator2(0, 2, 1, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);

    //Then: IsMapped returns false
    EXPECT_FALSE(translator2.IsMapped());

    //When: translate muliple blocks
    Translator translator3(0, 0, 2, 0, true, mockIVSAMap, mockIStripeMap, mockWBAllocator, mockITranslator, mockIArrayInfo);

    //Then: IsMapped throws an exception
    EXPECT_THROW(translator3.IsMapped(), POS_EVENT_ID);
}

} // namespace pos