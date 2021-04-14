#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mio_handler.h"

namespace pos
{
class MockMioHandler : public MioHandler
{
public:
    using MioHandler::MioHandler;
};

} // namespace pos
