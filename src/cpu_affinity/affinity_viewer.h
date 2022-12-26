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

#include <bitset>
#include <map>
#include <string>
#include <vector>

namespace pos
{
class AffinityManager;

class AffinityViewer
{
public:
    static void Print(void);
    static void Print(AffinityManager* affinityManager);

private:
    enum Role
    {
        REACTOR = 0,
        NONE,
        EVENT,
        USER_IO,
        SCHEDULER,
        GENERAL,
        META_SCHEDULER,
        META_IO,
        QOS,
        AIR,
        EVENT_REACTOR,
        DEBUG,
        ROLE_COUNT,
    };

    class Cpu
    {
    public:
        Cpu(void);
        Cpu(int id);
        void Print(void);
        void Register(int id, Role role);

    private:
        int id;
        std::bitset<ROLE_COUNT> roleBitmap;
        static const char* ROLE_DESCRIPTIONS[ROLE_COUNT];
    };

    class Socket
    {
    public:
        Socket(void);
        Socket(int id, int totalCpuCount);
        void Print(void);
        void RegisterRole(int coreId, Role role);

    private:
        int id;
        int cpuCount;
        int totalCpuCount;
        using CpuMap = std::map<int, Cpu>;
        using CpuMapIterator = CpuMap::iterator;
        CpuMap cpus;
    };

    class Numa
    {
    public:
        explicit Numa(AffinityManager* affinityManager);
        void Print(void);
        void RegisterEveryCpuRole(void);

    private:
        int socketCount;
        int totalCpuCount;
        std::vector<Socket> sockets;
        AffinityManager& affinityManager;

        void _RegisterReactorCpu(void);
        void _RegisterQoSCpu(void);
        void _RegisterEventCpu(void);
        void _RegisterUserIoCpu(void);
        void _RegisterSchedulerCpu(void);
        void _RegisterGeneralCpu(void);
        void _RegisterMetaSchedulerCpu(void);
        void _RegisterMetaCpu(void);
        void _RegisterAirCpu(void);
        void _RegisterEventReactorCpu(void);
        void _RegisterDebugCpu(void);
        void _RegisterCpuRole(int cpuId, Role role);
        void _RegisterCpuSetRole(cpu_set_t cpuSet, Role role);
    };
};
} // namespace pos
