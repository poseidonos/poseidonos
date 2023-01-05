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

#include "affinity_viewer.h"

#include <numa.h>

#include <iostream>

#include "src/cpu_affinity/affinity_manager.h"

using namespace std;
namespace pos
{
const char* AffinityViewer::Cpu::ROLE_DESCRIPTIONS[ROLE_COUNT] =
    {
        "Reactor",
        "None",
        "Event",
        "UserIO",
        "Scheduler",
        "General",
        "MetaScheduler",
        "MetaIO",
        "QOS",
        "AIR",
        "EventReactor",
        "Debug"};

AffinityViewer::Socket::Socket(void)
: id(0),
  cpuCount(0),
  totalCpuCount(0)
{
}

AffinityViewer::Socket::Socket(int id, int totalCpuCount)
: id(id),
  cpuCount(0),
  totalCpuCount(totalCpuCount)
{
    for (int cpuId = 0; cpuId < totalCpuCount; cpuId++)
    {
        int correspondingSocketId = numa_node_of_cpu(cpuId);
        if (correspondingSocketId == id)
        {
            cpuCount++;
            cpus[cpuId] = Cpu(cpuId);
        }
    }
}

void
AffinityViewer::Cpu::Print(void)
{
    cout << "CPU ID: " << id << "\tUsage:";
    for (int role = 0; role < ROLE_COUNT; role++)
    {
        if (roleBitmap.test(role))
        {
            cout << " " << ROLE_DESCRIPTIONS[role];
        }
    }
    cout << endl;
}

AffinityViewer::Cpu::Cpu(void)
: id(0)
{
    roleBitmap.set(NONE);
}

AffinityViewer::Cpu::Cpu(int id)
: id(id)
{
    roleBitmap.set(NONE);
}
void
AffinityViewer::Cpu::Register(int coreId, Role role)
{
    id = coreId;
    roleBitmap.set(role);
    roleBitmap.reset(NONE);
}

void
AffinityViewer::Socket::Print(void)
{
    cout << "Socket ID: " << id << ", " << cpuCount << "CPUs\n";
    cout << "===============================\n";
    for (CpuMapIterator iter = cpus.begin(); iter != cpus.end(); ++iter)
    {
        Cpu& targetCpu = iter->second;
        targetCpu.Print();
    }
    cout << "===============================\n";
}

void
AffinityViewer::Socket::RegisterRole(int coreId, Role role)
{
    cpus[coreId].Register(coreId, role);
}

void
AffinityViewer::Numa::Print(void)
{
    cout << "===============================\n";
    cout << socketCount << " Sockets in Current NUMA\n";
    cout << "===============================\n";
    for (int socketId = 0; socketId < socketCount; socketId++)
    {
        sockets[socketId].Print();
    }
    if (false == affinityManager.UseEventReactor())
    {
        uint32_t eventWorkerSocket = affinityManager.GetEventWorkerSocket();
        cout << "Event Worker Socket: " << eventWorkerSocket << endl;
        cout << "===============================\n";
    }
}

AffinityViewer::Numa::Numa(AffinityManager* affinityManager)
: socketCount(numa_num_configured_nodes()),
  totalCpuCount(numa_num_configured_cpus()),
  sockets(socketCount),
  affinityManager(*affinityManager)
{
    for (int socketId = 0; socketId < socketCount; socketId++)
    {
        sockets[socketId] = Socket(socketId, totalCpuCount);
    }
}

void
AffinityViewer::Numa::_RegisterReactorCpu(void)
{
    cpu_set_t reactorCpuSet = affinityManager.GetCpuSet(CoreType::REACTOR);
    _RegisterCpuSetRole(reactorCpuSet, REACTOR);
}

void
AffinityViewer::Numa::_RegisterQoSCpu(void)
{
    cpu_set_t reactorCpuSet = affinityManager.GetCpuSet(CoreType::QOS);
    _RegisterCpuSetRole(reactorCpuSet, QOS);
}

void
AffinityViewer::Numa::_RegisterCpuRole(int cpuId, Role role)
{
    int socketId = numa_node_of_cpu(cpuId);
    if (static_cast<uint32_t>(socketId) == INVALID_NUMA)
    {
        return;
    }

    sockets[socketId].RegisterRole(cpuId, role);
}

void
AffinityViewer::Numa::_RegisterCpuSetRole(cpu_set_t cpuSet, Role role)
{
    for (int cpuId = 0; cpuId < totalCpuCount; cpuId++)
    {
        if (CPU_ISSET(cpuId, &cpuSet))
        {
            _RegisterCpuRole(cpuId, role);
        }
    }
}

void
AffinityViewer::Numa::_RegisterEventCpu(void)
{
    cpu_set_t eventCpuSet = affinityManager.GetCpuSet(CoreType::EVENT_WORKER);
    _RegisterCpuSetRole(eventCpuSet, EVENT);
}

void
AffinityViewer::Numa::_RegisterMetaSchedulerCpu(void)
{
    cpu_set_t metaCpuSet = affinityManager.GetCpuSet(CoreType::META_SCHEDULER);
    _RegisterCpuSetRole(metaCpuSet, META_SCHEDULER);
}

void
AffinityViewer::Numa::_RegisterMetaCpu(void)
{
    cpu_set_t metaCpuSet = affinityManager.GetCpuSet(CoreType::META_IO);
    _RegisterCpuSetRole(metaCpuSet, META_IO);
}

void
AffinityViewer::Numa::_RegisterUserIoCpu(void)
{
    cpu_set_t userIoCpuSet = affinityManager.GetCpuSet(CoreType::UDD_IO_WORKER);
    _RegisterCpuSetRole(userIoCpuSet, USER_IO);
}

void
AffinityViewer::Numa::_RegisterSchedulerCpu(void)
{
    cpu_set_t schedulerCpuSet =
        affinityManager.GetCpuSet(CoreType::EVENT_SCHEDULER);
    _RegisterCpuSetRole(schedulerCpuSet, SCHEDULER);
}

void
AffinityViewer::Numa::_RegisterGeneralCpu(void)
{
    cpu_set_t generalCpuSet =
        affinityManager.GetCpuSet(CoreType::GENERAL_USAGE);
    _RegisterCpuSetRole(generalCpuSet, GENERAL);
}

void
AffinityViewer::Numa::_RegisterAirCpu(void)
{
    cpu_set_t airCpuSet =
        affinityManager.GetCpuSet(CoreType::AIR);
    _RegisterCpuSetRole(airCpuSet, AIR);
}

void
AffinityViewer::Numa::_RegisterEventReactorCpu(void)
{
    cpu_set_t eventReactorCpuSet =
        affinityManager.GetCpuSet(CoreType::EVENT_REACTOR);
    _RegisterCpuSetRole(eventReactorCpuSet, EVENT_REACTOR);
}

void
AffinityViewer::Numa::_RegisterDebugCpu(void)
{
    cpu_set_t debugCpu =
        affinityManager.GetCpuSet(CoreType::DEBUG);
    _RegisterCpuSetRole(debugCpu, DEBUG);
}

void
AffinityViewer::Numa::RegisterEveryCpuRole(void)
{
    _RegisterReactorCpu();
    _RegisterEventCpu();
    _RegisterUserIoCpu();
    _RegisterSchedulerCpu();
    _RegisterGeneralCpu();
    _RegisterMetaSchedulerCpu();
    _RegisterMetaCpu();
    _RegisterQoSCpu();
    _RegisterAirCpu();
    _RegisterEventReactorCpu();
    _RegisterDebugCpu();
}

void
AffinityViewer::Print(void)
{
    Print(AffinityManagerSingleton::Instance());
}

void
AffinityViewer::Print(AffinityManager* affinityManager)
{
    bool numaAvailable = (numa_available() != -1);
    if (numaAvailable)
    {
        Numa numa(affinityManager);
        numa.RegisterEveryCpuRole();
        numa.Print();
    }
    else
    {
        cout << "NUMA library is not available\n";
    }
}
} // namespace pos
