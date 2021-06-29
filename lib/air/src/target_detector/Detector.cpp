
#include "src/target_detector/Detector.h"

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <string>

void
detect::Detector::Process(void)
{
    std::string str_dir = "/proc/" + std::to_string(getpid()) + "/task";

    DIR* dp = opendir(str_dir.c_str());
    if (nullptr != dp)
    {
        for (;;)
        {
            struct dirent* item = readdir(dp);
            if (nullptr == item)
            {
                break;
            }

            std::string str_tid = item->d_name;
            if (str_tid.compare(".") == 0 || str_tid.compare("..") == 0)
            {
                continue;
            }
            _CreateNodeDataArray(std::stoi(str_tid));
        }
    }
    closedir(dp);

    _DeleteNodeDataArray();
}

void
detect::Detector::_CreateNodeDataArray(uint32_t tid)
{
    node_manager->CreateNodeDataArray(tid);
}

void
detect::Detector::_DeleteNodeDataArray(void)
{
    std::string str_dir = "/proc/" + std::to_string(getpid()) + "/task/";

    auto iter = node_manager->nda_map.begin();
    while (iter != node_manager->nda_map.end())
    {
        DIR* dp = opendir((str_dir + std::to_string(iter->first)).c_str());
        if (nullptr == dp)
        {
            if (true == node_manager->CanDeleteNodeDataArray(iter->second))
            {
                node_manager->DeleteNodeDataArray(iter->second);
                node_manager->nda_map.erase(iter++);
            }
            else
            {
                iter++;
            }
        }
        else
        {
            iter++;
        }
        closedir(dp);
    }
}
