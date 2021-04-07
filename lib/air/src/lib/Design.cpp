
#include "src/lib/Design.h"

int
lib_design::Subject::Attach(Observer* observer, uint32_t index)
{
    if (index < ARR_SIZE)
    {
        arr_observer[index] = observer;
        return 0;
    }

    return -3; // interface error
}
