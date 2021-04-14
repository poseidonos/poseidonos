#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_avg_compute.h"

namespace pos
{
class MockMovingAvgCompute : public MovingAvgCompute
{
public:
    using MovingAvgCompute::MovingAvgCompute;
};

} // namespace pos
