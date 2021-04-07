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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#pragma once
#include "mk/ibof_config.h"
#include "nlohmann/json.hpp"
#include "src/helper/json_helper.h"
#include "src/main/main_handler.h"

#if defined QOS_ENABLED_BE
#include "src/scheduler/event.h"
#endif

#include <map>
#include <string>

using namespace std;

using json = nlohmann::json;
namespace ibofos_cli
{
class RequestHandler
{
public:
    bool
    IsExit()
    {
        return isExit;
    }

    string CommandProcessing(char* msg);
    string TimedOut(char* msg);
    string PosBusy(char* msg);
    JsonElement POSInfo(string name = "info");
    string ScanDevice(json&, string rid);
    string ListDevice(json&, string rid);
    string AddDevice(json&, string rid);
    string RemoveDevice(json&, string rid);
    string StartDeviceMonitoring(json&, string rid);
    string StopDeviceMonitoring(json&, string rid);
    string DeviceMonitoringState(json&, string rid);

    string CreateArray(json&, string rid);
    string LoadArray(json&, string rid);
    string DeleteArray(json&, string rid);
    string ListArrayDevice(json&, string rid);
    string ArrayInfo(json&, string rid);

    string CreateVolume(json&, string rid);
    string DeleteVolume(json&, string rid);
    string MountVolume(json&, string rid);
    string UnmountVolume(json&, string rid);
    string ListVolume(json&, string rid);
    string UpdateVolumeQoS(json&, string rid);
    string RenameVolume(json&, string rid);
    string ResizeVolume(json&, string rid);
    string GetMaxVolumeCount(json&, string rid);
    string GetHostNQN(json&, string rid);

    string CondSignal(json&, std::string rid);
    // iBoFOS Handling
    string MountIbofos(json&, string rid);
    string UnmountIbofos(json&, string rid);
    string MountArray(json&, string rid);
    string UnmountArray(json&, string rid);
    // System Mount/Unmount, Total Capacity, Used Space
    string GetIbofosInfo(json&, string rid);
    string ExitIbofos(json&, string rid);

    string StopRebuilding(json&, string rid);
    string RebuildPerfImpact(json&, string rid);
    string ApplyLogFilter(json&, string rid);
    string SetLogLevel(json&, string rid);
    string GetLogLevel(json&, string rid);
    string LoggerInfo(json& doc, string rid);
    string Smart(json& doc, string rid);

    string GetVersion(json&, string rid);
    // string PassThroughNvmeAdminCommand(json &, string rid);

    string ListWBT(json&, string rid);
    string WBT(json&, string rid);

#if defined QOS_ENABLED_BE
    string UpdateEventWrrPolicy(json&, string rid);
    string ResetEventWrrPolicy(json&, string rid);
#endif
#if defined QOS_ENABLED_FE
    string UpdateVolumeMinimumPolicy(json&, string rid);
#endif

private:
#if (1 == IBOF_CONFIG_LIBRARY_BUILD)
    bool isValidCondSignal = true;
#else
    bool isValidCondSignal = false;
#endif

    bool isExit = false;
    ibofos::MainHandler mainHandler;
    typedef string (RequestHandler::*HANDLER)(json&, string);
    map<string, HANDLER> cmdDictionary = {
        {"SCANDEVICE", &RequestHandler::ScanDevice},
        {"LISTDEVICE", &RequestHandler::ListDevice},
        {"ADDDEVICE", &RequestHandler::AddDevice},
        {"REMOVEDEVICE", &RequestHandler::RemoveDevice},
        {"STARTDEVICEMONITORING", &RequestHandler::StartDeviceMonitoring},
        {"STOPDEVICEMONITORING", &RequestHandler::StopDeviceMonitoring},
        {"DEVICEMONITORINGSTATE", &RequestHandler::DeviceMonitoringState},
        {"CREATEARRAY", &RequestHandler::CreateArray},
        {"LOADARRAY", &RequestHandler::LoadArray},
        {"DELETEARRAY", &RequestHandler::DeleteArray},
        {"LISTARRAYDEVICE", &RequestHandler::ListArrayDevice},
        {"ARRAYINFO", &RequestHandler::ArrayInfo},
        {"CREATEVOLUME", &RequestHandler::CreateVolume},
        {"DELETEVOLUME", &RequestHandler::DeleteVolume},
        {"MOUNTVOLUME", &RequestHandler::MountVolume},
        {"UNMOUNTVOLUME", &RequestHandler::UnmountVolume},
        {"LISTVOLUME", &RequestHandler::ListVolume},
        {"UPDATEVOLUMEQOS", &RequestHandler::UpdateVolumeQoS},
        {"RENAMEVOLUME", &RequestHandler::RenameVolume},
        {"RESIZEVOLUME", &RequestHandler::ResizeVolume},
        {"GETMAXVOLUMECOUNT", &RequestHandler::GetMaxVolumeCount},
        {"GETHOSTNQN", &RequestHandler::GetHostNQN},
        {"CONDSIGNAL", &RequestHandler::CondSignal},
        {"MOUNTIBOFOS", &RequestHandler::MountIbofos},
        {"UNMOUNTIBOFOS", &RequestHandler::UnmountIbofos},
        {"MOUNTARRAY", &RequestHandler::MountArray},
        {"UNMOUNTARRAY", &RequestHandler::UnmountArray},
        {"GETIBOFOSINFO", &RequestHandler::GetIbofosInfo},
        {"EXITIBOFOS", &RequestHandler::ExitIbofos},
        {"STOPREBUILDING", &RequestHandler::StopRebuilding},
        {"REBUILDPERFIMPACT", &RequestHandler::RebuildPerfImpact},
        {"APPLYLOGFILTER", &RequestHandler::ApplyLogFilter},
        {"SETLOGLEVEL", &RequestHandler::SetLogLevel},
        {"GETLOGLEVEL", &RequestHandler::GetLogLevel},
        {"LOGGERINFO", &RequestHandler::LoggerInfo},
        {"SMART", &RequestHandler::Smart},
        {"GETVERSION", &RequestHandler::GetVersion},
    // {"PASSTHROUGHNVMEADMIN", &RequestHandler::PassThroughNvmeAdminCommand},
        {"LISTWBT", &RequestHandler::ListWBT},
        {"WBT", &RequestHandler::WBT},

#if defined QOS_ENABLED_BE
        {"UPDATEEVENTWRRPOLICY", &RequestHandler::UpdateEventWrrPolicy},
        {"RESETEVENTWRRPOLICY", &RequestHandler::ResetEventWrrPolicy},
#endif
#if defined QOS_ENABLED_FE
        {"UPDATEVOLUMEMINPOLICY", &RequestHandler::UpdateVolumeMinimumPolicy},
#endif
    };
#if defined QOS_ENABLED_BE
    map<string, ibofos::BackendEvent> eventDict = {
        {"flush", ibofos::BackendEvent_Flush},
        {"gc", ibofos::BackendEvent_GC},
        {"rebuild", ibofos::BackendEvent_UserdataRebuild},
        {"meta_rebuild", ibofos::BackendEvent_MetadataRebuild},
        {"metaio", ibofos::BackendEvent_MetaIO},
        {"fe_rebuild", ibofos::BackendEvent_FrontendIO}};
    ibofos::BackendEvent _GetEventId(string eventName);
#endif
};
}; // namespace ibofos_cli
