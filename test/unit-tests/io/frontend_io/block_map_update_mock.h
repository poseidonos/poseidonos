#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/frontend_io/block_map_update.h"
 
namespace pos {

class MockBlockMapUpdate : public BlockMapUpdate {
  public:
    using BlockMapUpdate::BlockMapUpdate;
    MOCK_METHOD(bool, Execute, (), (override));
};

}  // namespace pos
