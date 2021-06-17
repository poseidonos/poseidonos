#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/affinity_config_parser.h"

namespace pos
{
class MockAffinityConfigParser : public AffinityConfigParser
{
public:
    using AffinityConfigParser::AffinityConfigParser;
    MOCK_METHOD(bool, IsStringDescripted, (), (override));
    MOCK_METHOD(const CoreDescriptionArray&, GetDescriptions, (), (override));
};

} // namespace pos
