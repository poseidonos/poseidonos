#include "src/cpu_affinity/affinity_viewer.h"

#include <gtest/gtest.h>

namespace pos
{
// FIXME: Disabled to avoid invoking exit() on a host where the number of CPU cores is smaller than 10 [AWIBOF-3632]
TEST(AffinityViewer, DISABLED_Print_numaAvailable)
{
    // Given
    AffinityViewer affinityViewer;

    // When : call Print
    affinityViewer.Print();
    
    // Then : Do nothing
}

} // namespace pos
