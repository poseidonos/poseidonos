#pragma once

#include <unistd.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/ioat.h"
#include "spdk/stdinc.h"
#include "src/lib/singleton.h"

using SpdkNvmfStartCompletedCallback_t = std::function<int(void)>;

using namespace std;

namespace ibofos
{
class Spdk
{
public:
    Spdk();
    ~Spdk(void);

    bool Init(int argc, char** argv);
    void Finalize();

    static void SetStartCompletedHandler(SpdkNvmfStartCompletedCallback_t handler);
    static SpdkNvmfStartCompletedCallback_t GetStartCompletedHandler();

private:
    std::thread* spdkThread;
    static SpdkNvmfStartCompletedCallback_t startCompletedHandler;
    static std::atomic<bool> spdkInitialized;

    static void _InitWorker(int argc, char** argv);
    static void _AppStartedCallback(void* arg1);
    static int
    _AppParseCallback(int ch, char* arg)
    {
        return 0;
    }
    static void
    _AppUsageCallback(void)
    {
    }
};

using SpdkSingleton = Singleton<Spdk>;

} // namespace ibofos
