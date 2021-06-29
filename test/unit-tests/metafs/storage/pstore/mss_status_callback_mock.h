#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_status_callback.h"

namespace pos
{
class MockMssAioData : public MssAioData
{
public:
    using MssAioData::MssAioData;
};

class MockMssAioCbCxt : public MssAioCbCxt
{
public:
    using MssAioCbCxt::MssAioCbCxt;
};

} // namespace pos
