#include <numa.h>
#include <iostream>

#include "src/cpu_affinity/affinity_viewer.h"

using namespace std;
using namespace pos;

int main(void)
{
    AffinityViewer::Print();

    return 0;
}
