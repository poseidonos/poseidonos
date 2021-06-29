#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/block_alignment.h"

namespace pos
{
class MockBlockAlignment : public BlockAlignment
{
public:
    using BlockAlignment::BlockAlignment;
    MOCK_METHOD(uint32_t, GetBlockCount, (), (override));
    MOCK_METHOD(uint32_t, GetHeadSize, (), (override));
    MOCK_METHOD(uint32_t, GetTailSize, (), (override));
    MOCK_METHOD(bool, HasTail, (), (override));
    MOCK_METHOD(bool, HasHead, (), (override));
    MOCK_METHOD(uint64_t, AlignHeadLba, (uint32_t blockIndex, uint64_t originalLba), (override));
    MOCK_METHOD(uint32_t, GetDataSize, (uint32_t blockIndex), (override));
    MOCK_METHOD(uint64_t, GetHeadBlock, (), (override));
    MOCK_METHOD(uint64_t, GetTailBlock, (), (override));
    MOCK_METHOD(uint32_t, GetHeadPosition, (), (override));
};

} // namespace pos
