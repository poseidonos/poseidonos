#include <gmock/gmock.h>
#include <string>
#include <map>
#include "src/pbr/load/pbr_voting.h"

using namespace std;

namespace pbr
{
class MockPbrVoting : public PbrVoting
{
public:
    using PbrVoting::PbrVoting;
    MOCK_METHOD(void, Vote, (AteData*), (override));
    MOCK_METHOD((map<string, AteData*>), Poll, (), (override));
};

} // namespace pbr
