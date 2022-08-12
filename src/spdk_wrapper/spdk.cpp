/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "spdk.h"

#include <string>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/spdk_wrapper/accel_engine_api.h"

using namespace std;

namespace pos
{
std::atomic<bool> Spdk::spdkInitialized;


void
Spdk::_AppStartedCallback(void* arg1)
{
    if (getenv("MEMZONE_DUMP") != NULL)
    {
        spdk_memzone_dump(stdout);
        fflush(stdout);
    }

    spdk_log_set_level(SPDK_LOG_WARN);
    spdk_log_set_print_level(SPDK_LOG_WARN);
    spdk_log_set_flag("all");
    spdk_log_clear_flag("reactor");
    spdk_log_set_flag("bdev");
    spdk_log_set_flag("bdev_nvme");
    spdk_log_set_flag("nvme");
    spdk_log_set_flag("bdev_malloc");
    spdk_log_set_flag("bdev_ibof");

    AccelEngineApi::Initialize();

    spdkInitialized = true;

    cout << "poseidonos started" << endl;
}

bool
Spdk::Init(int argc, char** argv)
{
    if (spdkInitialized == true)
    {
        cout << "SPDK is already initialized" << endl;
        return true;
    }

    spdkThread = new std::thread(_InitWorker, argc, argv);
    while (spdkInitialized == false)
    {
        usleep(1);
    }
    return true;
}

void
Spdk::_InitWorker(int argc, char** argv)
{
    int rc = 0;
    struct spdk_app_opts opts = {};

    /* default value in opts */
    spdk_app_opts_init(&opts, sizeof(opts));
    opts.name = "ibof_nvmf";
    opts.mem_channel = -1;
    AffinityManager& affinityManager = *AffinityManagerSingleton::Instance();
    std::string reactorMaskString = affinityManager.GetReactorCPUSetString();
    opts.reactor_mask = reactorMaskString.c_str();
    opts.main_core = affinityManager.GetMasterReactorCore();
    opts.print_level = SPDK_LOG_INFO;

    rc = spdk_app_parse_args(argc,
        argv,
        &opts,
        "",
        NULL,
        Spdk::_AppParseCallback,
        Spdk::_AppUsageCallback);
    if (rc != SPDK_APP_PARSE_ARGS_SUCCESS)
    {
        exit(rc);
    }

    /* Blocks until the application is exiting */
    rc = spdk_app_start(&opts, Spdk::_AppStartedCallback, NULL);
}

void
Spdk::Finalize()
{
    spdk_app_stop(0);
    if (spdkThread)
    {
        spdkThread->join();
    }
    spdk_app_fini();

    spdkInitialized = false;
    cout << "SPDK app finalized" << endl;
}

Spdk::Spdk()
: spdkThread(nullptr)
{
}

Spdk::~Spdk(void)
{
    try
    {
        Finalize();
    }
    catch (...)
    {
        cout << "fail to finalize SPDK";
    }
}

} // namespace pos
