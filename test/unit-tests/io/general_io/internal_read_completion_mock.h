#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/internal_read_completion.h"

namespace pos
{
class MockInternalReadCompletion : public InternalReadCompletion
{
public:
    using InternalReadCompletion::InternalReadCompletion;
};

} // namespace pos
