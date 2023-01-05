#include <gtest/gtest.h>
#include "src/debug_lib/debug_info_maker.h"
#include "src/debug_lib/debug_info_maker.hpp"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"
 
namespace pos {

class DummyDebugInfo : public DebugInfoInstance
{

};

class DummyDebugInfoMaker : public DebugInfoMaker<DummyDebugInfo>
{
public:
    virtual void MakeDebugInfo(DummyDebugInfo& obj) final
    {
    }
};

TEST(DebugInfoInstance, RegisterDebugInfoInstance1) {
    DummyDebugInfo dummyDebugInfo;
    dummyDebugInfo.RegisterDebugInfoInstance("dummy");
}

}  // namespace pos

namespace pos {

TEST(DebugInfoMaker, DebugInfoMaker_) {
    DummyDebugInfoMaker dummyDebugInfoMaker;
    dummyDebugInfoMaker.RegisterDebugInfo("dummy", 100, true);
    dummyDebugInfoMaker.AddDebugInfo();
}

TEST(DebugInfoMaker, MakeDebugInfo_) {
    DummyDebugInfoMaker dummyDebugInfoMaker;
    dummyDebugInfoMaker.RegisterDebugInfo("dummy", 100, true);
    dummyDebugInfoMaker.AddDebugInfo();
}

TEST(DebugInfoMaker, MakeDebugInfoFail) {
    DummyDebugInfoMaker dummyDebugInfoMaker;
    EXPECT_DEATH(dummyDebugInfoMaker.AddDebugInfo(), "");
}

}  // namespace pos
