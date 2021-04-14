#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/frontend_io/read_completion_for_partial_write.h"

namespace pos
{
class MockReadCompletionForPartialWrite : public ReadCompletionForPartialWrite
{
public:
    using ReadCompletionForPartialWrite::ReadCompletionForPartialWrite;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

class MockCopyParameter : public CopyParameter
{
public:
    using CopyParameter::CopyParameter;
};
} // namespace pos
