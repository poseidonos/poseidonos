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

#include <execinfo.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/main/poseidonos.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/version_provider.h"
#include "src/trace/trace_exporter.h"
#include "src/trace/otlp_factory.h"
#include "src/lib/singleton.h"

#if defined UNVME_BUILD
#include "src/spdk_wrapper/spdk.h"
#endif

#include <air/Air.h>

#include "mk/ibof_config.h"

#define SEM_KEY (0xC0021B0F)
#define NR_PROC_CONCURRENT_ALLOWED (1)

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short int* arr;
};

void
PreventDualExecution(int nrProc);
int
CheckPrevileges(void);
#if IBOF_CONFIG_LIBRARY_BUILD == 1

int argc = 1;
char argv[] = {"./bin/poseidonos"};

extern "C"
{
    extern int optind;
    int
    intialize_library()
    {
        air_initialize(0);
        air_activate();

        /*optind used in getopt.c of glic needs to be zero, 
      because, system already has used up this value for parsing fio's argument */
        optind = 0;
        char* argvPtr = argv;

        pos::Poseidonos _pos;
        _pos.Init(argc, &argvPtr);

        _pos.Run();
        _pos.Terminate();

        return 0;
    }
}
#else

int
main(int argc, char* argv[])
{
    PreventDualExecution(NR_PROC_CONCURRENT_ALLOWED);
    int ret = CheckPrevileges();
    if (ret != 0)
    {
        return ret;
    }

    pos::Poseidonos _pos;
    ret = _pos.Init(argc, argv);
    if (ret != 0)
    {
        return ret;
    }
    
    _pos.Run();
    _pos.Terminate();

    return 0;
}
#endif

void
PreventDualExecution(int nrProc)
{
    int semCount = 1;
    int semFlags = IPC_EXCL | IPC_CREAT | 0660;
    int semid = semget(static_cast<key_t>(SEM_KEY), semCount, semFlags);
    if (-1 == semid)
    {
        if (EEXIST == errno)
        {
            /* get existent semaphore for SEM_KEY */
            semCount = 0;
            semFlags = 0;
            semid = semget(static_cast<key_t>(SEM_KEY), semCount, semFlags);
        }
        else
        {
            std::cout << "Failed to get semaphore: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
    else
    {
        /* initialize new semaphore */
        union semun semUnion;
        semUnion.val = nrProc;
        int semIndex = 0;
        if (-1 == semctl(semid, semIndex, SETVAL, semUnion))
        {
            std::cout << "Failed to initiate value for semaphore: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }

    {
        /* try to decrease semaphore count if none zero */
        struct sembuf mysemOpen = {0, -1, IPC_NOWAIT | SEM_UNDO};
        if (-1 == semop(semid, &mysemOpen, 1))
        {
            std::cout << "All Arrays already exist up to limit: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
}

int
CheckPrevileges(void)
{
    setvbuf(stdout, (char*)NULL, _IONBF, 0);
    if (geteuid() != 0)
    {
        std::cout << "Please run as root\n";
        return -1;
    }
    return 0;
}
