#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/mfs_state_procedure.h"

namespace pos
{
class MockMfsStateProcedure : public MfsStateProcedure
{
public:
    using MfsStateProcedure::MfsStateProcedure;
};

} // namespace pos
