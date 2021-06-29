#include "src/cpu_affinity/affinity_viewer.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(AffinityViewer, Print_numaAvailable)
{
    // Given
    AffinityViewer affinityViewer;

    // When : call Print
    affinityViewer.Print();
    
    // Then : Do nothing
}

} // namespace pos
