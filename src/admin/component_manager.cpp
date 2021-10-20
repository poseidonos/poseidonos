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

#include "src/admin/component_manager.h"

#include <stdio.h>

#include <fstream>
#include <typeinfo>

#include "iostream"
#include "string"
namespace pos
{
ComponentManager::ComponentManager(void)
: cpuTemperature(0),
  sumCpuTemperature(0),
  thermalZoneCount(0),
  fileOpened(false)
{
}
ComponentManager::~ComponentManager(void)
{
}
void
ComponentManager::_CalculateAvgTemp(uint64_t temp)
{
    if (thermalZoneCount != 0)
        cpuTemperature = temp / thermalZoneCount;
    else
        cpuTemperature = 0;
}
int
ComponentManager::FindCpuTemperature(void)
{
    int i = 0;
    for (i = 0; i < ALLOWED_THERMAL_ZONES; i++)
    {
        filePath = TEMPERATURE_COMMAND_1 + std::to_string(i) + TEMPERATURE_COMMAND_2;
        std::ifstream file(filePath);
        if (file.is_open())
        {
            fileOpened = true;
            std::string line;
            while (std::getline(file, line))
            {
                thermalZoneCount++;
                int temperature = atoi(line.c_str());
                sumCpuTemperature = sumCpuTemperature + temperature;
            }
            file.close();
        }
        else
        {
            break;
        }
    }

    _CalculateAvgTemp(sumCpuTemperature);
    return cpuTemperature / MILLI;
}

} // namespace pos
