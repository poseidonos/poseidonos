#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_q.h"

namespace pos
{
template<typename T>
class MockMetaFsIoQ : public MetaFsIoQ<T>
{
public:
    using MetaFsIoQ<T>::MetaFsIoQ;
    MOCK_METHOD(void, Init, (const char* qName, uint32_t numEntries), (override));
    MOCK_METHOD(bool, IsAllQEmpty, (), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
    MOCK_METHOD(uint32_t, GetItemCnt, (), (override));
    MOCK_METHOD(void, SetWeightFactor, (uint32_t weight), (override));
    MOCK_METHOD(uint32_t, GetWeightFactor, (), (override));
    MOCK_METHOD(bool, Enqueue, (T obj), (override));
    MOCK_METHOD(T, Dequeue, (), (override));
    MOCK_METHOD(void, CleanQEntry, (), (override));
};

} // namespace pos
