#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mbr/mbr_info.h"

namespace pos
{
class MockdeviceInfo : public deviceInfo
{
public:
    using deviceInfo::deviceInfo;
};

class MockArrayBootRecord : public ArrayBootRecord
{
public:
    using ArrayBootRecord::ArrayBootRecord;
};

class MockmasterBootRecord : public masterBootRecord
{
public:
    using masterBootRecord::masterBootRecord;
};

} // namespace pos
