#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/merged_io.h"

namespace pos
{
class MockMergedIO : public MergedIO
{
public:
    using MergedIO::MergedIO;
};

} // namespace pos
