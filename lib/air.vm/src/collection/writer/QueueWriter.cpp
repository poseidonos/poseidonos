
#include "src/collection/writer/QueueWriter.h"

void
collection::QueueWriter::_UpdateRand(void)
{
    mersenne.seed(std::rand());
}
