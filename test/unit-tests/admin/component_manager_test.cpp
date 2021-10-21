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
    int foundTemperature = -1;
    bool expectedFileOpen = true;
    int temperature = componentManager.FindCpuTemperature();
}

TEST(ComponentManager, _CalculateAvgTemp_)
{
}

} // namespace pos
