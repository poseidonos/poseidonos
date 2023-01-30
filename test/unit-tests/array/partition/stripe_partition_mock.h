#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/partition/stripe_partition.h"

namespace pos
{
class MockStripePartition : public StripePartition
{
public:
    using StripePartition::StripePartition;
    MOCK_METHOD(int, Create, (uint64_t startLba, uint64_t lastLba, uint64_t totalNvmBlks), (override));
    MOCK_METHOD(void, RegisterService, (IPartitionServices* const svc), (override));
    MOCK_METHOD(int, Translate, (list<PhysicalEntry>& pel, const LogicalEntry& le), (override));
    MOCK_METHOD(int, GetParityList, (list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(int, ByteTranslate, (PhysicalByteAddr& dst, const LogicalByteAddr& src), (override));
    MOCK_METHOD(int, ByteConvert, (list<PhysicalByteWriteEntry> &dst, const LogicalByteWriteEntry &src), (override));
    MOCK_METHOD(bool, IsByteAccessSupported, (), (override));
    MOCK_METHOD(RaidState, GetRaidState, (), (override));
    MOCK_METHOD(int, GetRecoverMethod, (UbioSmartPtr ubio, RecoverMethod& out), (override));
    MOCK_METHOD(unique_ptr<RebuildContext>, GetRebuildCtx, (const vector<IArrayDevice*>& fault), (override));
    MOCK_METHOD(unique_ptr<RebuildContext>, GetQuickRebuildCtx, (const QuickRebuildPair& rebuildPair), (override));
    MOCK_METHOD(RaidType, GetRaidType, (), (override));
    MOCK_METHOD(void, _SetRebuildPair, (const vector<IArrayDevice*>& fault, RebuildPairs& rp), (override));
    MOCK_METHOD(void, _SetQuickRebuildPair, (const QuickRebuildPair& quickRebuildPair, RebuildPairs& rp, RebuildPairs& backupRp), (override));
};

} // namespace pos
