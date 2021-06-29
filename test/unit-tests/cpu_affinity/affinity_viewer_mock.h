#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/affinity_viewer.h"

namespace pos
{
class MockAffinityViewer : public AffinityViewer
{
public:
    using AffinityViewer::AffinityViewer;
};

} // namespace pos
