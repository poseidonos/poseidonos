#include "src/admin/component_manager.h"

#include <gtest/gtest.h>

#include <fstream>
#include <string>
namespace pos
{
TEST(ComponentManager, ComponentManager_Contsructor_One_Stack)
{
    ComponentManager componentManager();
}

TEST(ComponentManager, ComponentManager_Contsructor_One_Heap)
{
    ComponentManager* componentManager = new ComponentManager();
    delete componentManager;
}
TEST(ComponentManager, FindCpuTemperature_FileOpen)
{
    std::string filePath = "/sys/class/thermal/thermal_zone1/temp";

    ComponentManager componentManager;
    // std::ifstream file(filePath);
    int foundTemperature = -1;
    bool expectedFileOpen = true;
    // bool originalFileOpen = file.is_open();
    // as valid file name is passed file open should be true;
    bool originalFileOpen = componentManager.FindCpuTemperature();
    // ASSERT_EQ(expectedFileOpen, originalFileOpen);
}

TEST(ComponentManager, GetCpuTemperature_Initial_Zero)
{
    ComponentManager componentManager;
    uint64_t expected = 0;
    uint64_t actual = componentManager.GetCpuTemperature();
    ASSERT_EQ(expected, actual);
}

TEST(ComponentManager, _CalculateAvgTemp_)
{
}

} // namespace pos
