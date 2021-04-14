
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
            _CreateThread(std::stoi(str_tid));
        }
    }
    closedir(dp);

    _DeleteThread();
}

int
detect::Detector::_CreateThread(uint32_t tid)
{
    // 0 -> already create
    // 1 -> create complete
    return node_manager->CreateThread(tid);
}

int
detect::Detector::_DeleteThread(void)
{
    std::string str_dir = "/proc/" + std::to_string(getpid()) + "/task/";
    std::map<uint32_t, node::ThreadArray>::iterator it;

    it = node_manager->thread_map.begin();
    while (it != node_manager->thread_map.end())
    {
        DIR* dp = opendir((str_dir + std::to_string(it->first)).c_str());
        if (nullptr == dp)
        {
            if (true == node_manager->CanDelete(&(it->second)))
            {
                node_manager->DeleteThread(&(it->second));
                node_manager->thread_map.erase(it++);
            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
        closedir(dp);
    }
    return 1;
}
