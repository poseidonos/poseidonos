#include <numa.h>
#include <iostream>

#include "src/io/general_io/affinity_viewer.h"

using namespace std;
using namespace ibofos;

int main(void)
{
    AffinityViewer::Print();

    return 0;
}
