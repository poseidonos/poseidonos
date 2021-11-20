#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/helper/string/string_checker.h"

class MockStringChecker : public StringChecker
{
public:
    using StringChecker::StringChecker;
};
