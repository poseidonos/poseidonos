#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/main/poseidonos.h"

namespace pos
{
class MockIbofos : public Poseidonos
{
public:
    using Poseidonos::Poseidonos;
};

} // namespace pos
