#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/partition/stripe_partition.h"

namespace pos
{
class MockStripePartition : public StripePartition
{
public:
    using StripePartition::StripePartition;
    MOCK_METHOD(int, Create, (uint64_t startLba, uint32_t segCnt, uint64_t totalNvmBlks), (override));
    MOCK_METHOD(int, Translate, (list<PhysicalEntry> & dst, const LogicalEntry& src), (override));
    MOCK_METHOD(int, GetParityList, (list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(int, ByteTranslate, (PhysicalByteAddr & dst, const LogicalByteAddr& src), (override));
    MOCK_METHOD(int, ByteConvert, (list<PhysicalByteWriteEntry> & dst, const LogicalByteWriteEntry& src), (override));
    MOCK_METHOD(bool, IsByteAccessSupported, (), (override));
    MOCK_METHOD(int, GetRecoverMethod, (UbioSmartPtr ubio, RecoverMethod& out), (override));
    MOCK_METHOD(unique_ptr<RebuildContext>, GetRebuildCtx, (ArrayDevice * fault), (override));
};

} // namespace pos
