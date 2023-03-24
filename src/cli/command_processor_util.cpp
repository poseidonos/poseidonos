#include "src/array/array.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/cli_server.h"
#include "src/cli/command_processor.h"
#include "src/cli/request_handler.h"
#include "src/device/device_manager.h"
#include "src/event/event_manager.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/array_config.h"
#include "src/include/nvmf_const.h"
#include "src/include/poseidonos_interface.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/logger/logger.h"
#include "src/logger/preferences.h"
#include "src/master_context/version_provider.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/resource_checker/smart_collector.h"
#include "src/restore/restore_manager.h"
#include "src/sys_info/space_info.h"

pos::BackendEvent
CommandProcessor::_GetEventId(std::string eventName)
{
    map<string, pos::BackendEvent> eventDict = {
        {"flush", pos::BackendEvent_Flush},
        {"gc", pos::BackendEvent_GC},
        {"rebuild", pos::BackendEvent_UserdataRebuild},
        {"meta_rebuild", pos::BackendEvent_MetadataRebuild},
        {"journalio", pos::BackendEvent_JournalIO},
        {"metaio", pos::BackendEvent_MetaIO},
        {"flushmap", pos::BackendEvent_FlushMap},
        {"fe_rebuild", pos::BackendEvent_FrontendIO}};

    auto search = eventDict.find(eventName);
    if (search != eventDict.end())
    {
        return (search->second);
    }
    return (pos::BackendEvent_Unknown);
}

std::string
CommandProcessor::_GetRebuildImpactString(uint8_t impact)
{
    switch (impact)
    {
        case PRIORITY_HIGH:
            return "high";

        case PRIORITY_MEDIUM:
            return "medium";

        case PRIORITY_LOW:
            return "low";

        default:
            return "unknown";
    }
}

void
CommandProcessor::_SetEventStatus(int eventId, grpc_cli::Status* status, std::string message /*=""*/)
{
    std::string eventName = "";
    std::string description = "";
    std::string cause = "";
    std::string solution = "";

    auto event_info = eventManager.GetEventInfo();
    auto it = event_info->find(eventId);
    if (it != event_info->end())
    {
        eventName = it->second.GetEventName();
        description = it->second.GetDescription();
        cause = it->second.GetCause();
        solution = it->second.GetSolution();
    }

    status->set_code(eventId);
    status->set_event_name(eventName);
    status->set_description(description);
    status->set_cause(cause);
    status->set_solution(solution);

    if (message != "")
    {
        status->set_message(message);
    }
}

void
CommandProcessor::_SetPosInfo(grpc_cli::PosInfo* posInfo)
{
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    posInfo->set_version(version);
}

std::string
CommandProcessor::_GetGCMode(pos::IGCInfo* gc, std::string arrayName)
{
    if (arrayName == "")
    {
        return "N/A";
    }

    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return "N/A";
    }

    IContextManager* iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName);
    GcCtx* gcCtx = iContextManager->GetGcCtx();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        return "N/A";
    }

    GcMode gcMode = gcCtx->GetCurrentGcMode();

    std::string strGCMode;

    if (gcMode == GcMode::MODE_URGENT_GC)
    {
        strGCMode = "urgent";
    }
    else if (gcMode == GcMode::MODE_NORMAL_GC)
    {
        strGCMode = "normal";
    }
    else
    {
        strGCMode = "none";
    }

    return strGCMode;
}

void
CommandProcessor::_FillSmartData(
    struct spdk_nvme_health_information_page* payload,
    grpc_cli::SmartLog* data)
{
    char cString[128];

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.available_spare ? "WARNING" : "OK");
    string availableSpareSpace(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.device_reliability ? "WARNING" : "OK");
    string temperature(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.device_reliability ? "WARNING" : "OK");
    string deviceReliability(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.read_only ? "Yes" : "No");
    string readOnly(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.volatile_memory_backup ? "WARNING" : "OK");
    string volatileMemoryBackup(cString);

    snprintf(cString, sizeof(cString), "%dC", (int)payload->temperature - 273);
    string currentTemperature(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->available_spare);
    string availableSpare(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->available_spare_threshold);
    string availableSpareThreshold(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->percentage_used);
    string lifePercentageUsed(cString);

    _PrintUint128Dec(payload->data_units_read, cString, sizeof(cString));
    string dataUnitsRead(cString);

    _PrintUint128Dec(payload->data_units_written, cString, sizeof(cString));
    string dataUnitsWritten(cString);

    _PrintUint128Dec(payload->host_read_commands, cString, sizeof(cString));
    string hostReadCommands(cString);

    _PrintUint128Dec(payload->host_write_commands, cString, sizeof(cString));
    string hostWriteCommands(cString);

    _PrintUint128Dec(payload->controller_busy_time, cString, sizeof(cString));
    string controllerBusyTime(cString);

    _PrintUint128Dec(payload->power_cycles, cString, sizeof(cString));
    string powerCycles(cString);

    _PrintUint128Dec(payload->power_on_hours, cString, sizeof(cString));
    string powerOnHours(cString);

    _PrintUint128Dec(payload->unsafe_shutdowns, cString, sizeof(cString));
    string unsafeShutdowns(cString);

    _PrintUint128Dec(payload->media_errors, cString, sizeof(cString));
    string unrecoverableMediaErrors(cString);

    _PrintUint128Dec(payload->num_error_info_log_entries, cString, sizeof(cString));
    string lifetimeErrorLogEntries(cString);

    snprintf(cString, sizeof(cString), "%um", payload->warning_temp_time);
    string warningTemperatureTime(cString);

    snprintf(cString, sizeof(cString), "%um", payload->critical_temp_time);
    string criticalTemperatureTime(cString);

    data->set_availablesparespace(availableSpareSpace);
    data->set_temperature(temperature);
    data->set_devicereliability(deviceReliability);
    data->set_readonly(readOnly);
    data->set_volatilememorybackup(volatileMemoryBackup);
    data->set_currenttemperature(currentTemperature);
    data->set_availablespare(availableSpare);
    data->set_availablesparethreshold(availableSpareThreshold);
    data->set_lifepercentageused(lifePercentageUsed);
    data->set_dataunitsread(dataUnitsRead);
    data->set_dataunitswritten(dataUnitsWritten);
    data->set_hostreadcommands(hostReadCommands);
    data->set_hostwritecommands(hostWriteCommands);
    data->set_controllerbusytime(controllerBusyTime);
    data->set_powercycles(powerCycles);
    data->set_poweronhours(powerOnHours);
    data->set_unsafeshutdowns(unsafeShutdowns);
    data->set_unrecoverablemediaerrors(unrecoverableMediaErrors);
    data->set_lifetimeerrorlogentries(lifetimeErrorLogEntries);
    data->set_warningtemperaturetime(warningTemperatureTime);
    data->set_criticaltemperaturetime(criticalTemperatureTime);

    for (int i = 0; i < 8; i++)
    {
        if (payload->temp_sensor[i] != 0)
        {
            snprintf(cString, sizeof(cString), "%dC", (int)payload->temp_sensor[i] - 273);
            string* temperature = data->add_temperaturesensor();
            *temperature = cString;
        }
    }
}

void
CommandProcessor::_PrintUint128Hex(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        snprintf(s, n, "0x%llX%016llX", hi, lo);
    }
    else
    {
        snprintf(s, n, "0x%llX", lo);
    }
}

void
CommandProcessor::_PrintUint128Dec(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        _PrintUint128Hex(v, s, n);
    }
    else
    {
        snprintf(s, n, "%llu", (unsigned long long)lo);
    }
}

CommandProcessor::BiosInfo
CommandProcessor::_GetBiosInfo()
{
    const std::string getBiosVersionCmd = "dmidecode -s bios-version";
    const std::string getBiosVendorCmd = "dmidecode -s bios-vendor";
    const std::string getBiosReleaseDateCmd = "dmidecode -s bios-release-date";

    BiosInfo bios;
    bios.version = _ExecuteLinuxCmd(getBiosVersionCmd);
    bios.vendor = _ExecuteLinuxCmd(getBiosVendorCmd);
    bios.releaseDate = _ExecuteLinuxCmd(getBiosReleaseDateCmd);

    return bios;
}

CommandProcessor::SystemInfo
CommandProcessor::_GetSystemInfo()
{
    const std::string getSystemManufacturerCmd = "dmidecode -s system-manufacturer";
    const std::string getSystemProductNameCmd = "dmidecode -s system-product-name";
    const std::string getSystemSerialNumberCmd = "dmidecode -s system-serial-number";
    const std::string getSystemUuidCmd = "dmidecode -s system-uuid";

    SystemInfo system;
    system.manufacturer = _ExecuteLinuxCmd(getSystemManufacturerCmd);
    system.productName = _ExecuteLinuxCmd(getSystemProductNameCmd);
    system.serialNumber = _ExecuteLinuxCmd(getSystemSerialNumberCmd);
    system.uuid = _ExecuteLinuxCmd(getSystemUuidCmd);

    return system;
}

CommandProcessor::BaseboardInfo
CommandProcessor::_GetBaseboardInfo()
{
    const std::string getBaseboardManufacturerCmd = "dmidecode -s baseboard-manufacturer";
    const std::string getBaseboardProductNameCmd = "dmidecode -s baseboard-product-name";
    const std::string getBaseboardSerialNumberCmd = "dmidecode -s baseboard-serial-number";
    const std::string getBaseboardVersionCmd = "dmidecode -s baseboard-version";

    BaseboardInfo baseboard;
    baseboard.manufacturer = _ExecuteLinuxCmd(getBaseboardManufacturerCmd);
    baseboard.productName = _ExecuteLinuxCmd(getBaseboardProductNameCmd);
    baseboard.serialNumber = _ExecuteLinuxCmd(getBaseboardSerialNumberCmd);
    baseboard.version = _ExecuteLinuxCmd(getBaseboardVersionCmd);

    return baseboard;
}

CommandProcessor::ProcessorInfo
CommandProcessor::_GetProcessorInfo()
{
    const std::string getProcessorManufacturerCmd = "dmidecode -s processor-manufacturer";
    const std::string getProcessorVersioneCmd = "dmidecode -s processor-version";
    const std::string getProcessorFrequencyCmd = "dmidecode -s processor-frequency";

    ProcessorInfo processor;
    processor.manufacturer = _ExecuteLinuxCmd(getProcessorManufacturerCmd);
    processor.version = _ExecuteLinuxCmd(getProcessorVersioneCmd);
    processor.frequency = _ExecuteLinuxCmd(getProcessorFrequencyCmd);

    return processor;
}

std::string
CommandProcessor::_ExecuteLinuxCmd(std::string command)
{
    char buffer[MAX_LINUX_CMD_LENGTH];
    string result = "";
    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe)
    {
        return "popen failed!";
    }

    while (!feof(pipe))
    {
        if ((fgets(buffer, MAX_LINUX_CMD_LENGTH, pipe) != NULL))
            result += buffer;
    }

    result.erase(std::remove(result.begin(), result.end(), '\n'),
        result.end());
    pclose(pipe);

    return result;
}

bool
CommandProcessor::_IsValidIpAddress(const std::string& ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool
CommandProcessor::_IsValidFile(const std::string& path)
{
    std::string extension = path.substr(path.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c)
        { return std::tolower(c); });

    if (extension != "yaml")
    {
        return false;
    }

    if (FILE* file = fopen(path.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }

    return false;
}

int
CommandProcessor::_HandleInputVolumes(
    const string arrayName,
    const RepeatedPtrField<QosVolumeNameParam>& volumes,
    std::vector<string>& volumeNames,
    std::vector<std::pair<string, uint32_t>>& validVolumes)
{
    int validVol = -1;
    volumeNames.clear();
    validVolumes.clear();

    if (0 == arrayName.compare(""))
    {
        return EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
    }

    for (int i = 0; i < volumes.size(); i++)
    {
        string volName = volumes[i].volumename();
        volumeNames.push_back(volName);
    }

    IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (nullptr == volMgr)
    {
        return EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
    }

    for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
    {
        validVol = volMgr->CheckVolumeValidity(*vol);
        if (EID(SUCCESS) != validVol)
        {
            return validVol;
        }
        else
        {
            IVolumeInfoManager* volInfoMgr =
                VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
            validVolumes.push_back(std::make_pair(*vol, volInfoMgr->GetVolumeID(*vol)));
        }
    }
    return EID(SUCCESS);
}
