
#include "src/collection/writer/Writer.h"

void
collection::Writer::InformInit(lib::AccData* data)
{
    if (nullptr == data)
    {
        return;
    }
    data->need_erase = 1;
}
