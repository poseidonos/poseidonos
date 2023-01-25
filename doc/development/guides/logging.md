- [Introduction](#introduction)
- [Definitions](#definitions)
- [PoseidonOS EventID](#poseidonos-eventid)
- [PoseidonOS Log Entry](#poseidonos-log-entry)
- [Log Message](#log-message)
- [Log File](#log-file)

# Introduction
This document describes a comprehensive logging guideline for PoseidonOS contributors with the following contents. First, it contains the definitions and formats of PoseidonOS event log. Second, it explains how to add your event log to the PoseidonOS source code.

# Definitions
PoseidonOS uses concepts defined in [Common Event Expression Architecture Overview Version 0.5](http://cee.mitre.org/docs/CEE_Architecture_Overview-v0.5.pdf), which is a standard of event log. For detailed information, please refere to the document. 

1. **Event**: an event is a single occurence within an environment, usually involving and attempted state change. An event usually includes a notion of time, the occurrence, and any details the explicitly pertain to the event or environment that may help explain or understand the event's causes or effects.

2. **Event Category**: event category groups events based upon one or more event categorization methodologies. Example methodologies include organization based upon what happened during the event, the involved parties, device types impacted, etc. 

3. **Event Field**: an event field describes one characteristic of an event. Examples of an event field include date, time, source IP, user identification, and host identification.

4. **Event Record**: an event record is a collection of event fields that, together, describe a single event. Terms synonymous to event record include "audit record" and "log entry".

5. **Log**: a log is a collection of event records. Terms such as "data log," "activity log," "audit log," "audit trail," "log file," and "event log" are often used to mean the same thing as log.

6. **Audit**: an audit is the process of evaluating logs within an environment (e.g., within an electronic system). The typical goal of an audit is to assess the overall status or identify any notable or problematic activity.

7. **Recording**: Recording is the act of creating an event record comprised of the event fields associated with a single event.

8. **Logging**: Logging is the act of collecting event records into logs. Examples of logging include storing log entries into a text log file, or storing audit record data in binary files or databases.

# PoseidonOS Log Entry
PoseidonOS event log can be displayed in both JSON and plain-text forms. You can set the format via the configuration file (e.g., /etc/pos/pos.conf) or CLI ($poseidonos-cli logger set-preference).
When PoseidonOS records event logs in JSON form, it is called structured logging. Structured logging will provide a deeper insight into the PoseidonOS system; you can utilize the structured event logs to monitor and analyze the system.

## Example

### Plain-text form

[datetime][process_id][thread_id][pos_id][event_id][level] event_name - MESSAGE because CAUSE, solution:SOLUTION, variables:COMMMA_SEPARATED_VARIABLES, source: FILE:LINE FUNCTION(), pos_version: VERSION

```
[2022-03-04 15:56:08.557557315][23198][23302][78442531][1207][ info  ]     CLI_CLIENT_DISCONNECTED - A client has been disconnected., cause: , solution:, variables:fd:655, source: cli_server.cpp:168 RemoveClient(), pos_version: v0.10.6
```

### Event Level
PoseidonOS has the follwing event level structure:

#### Severity ####
debug < info < trace < warning < error < critical

#### Description
- **Debug**: This event describes the lowest-level state changes of PoseidonOS, of which the record is mainly referred by developers for debugging purposes.
- **Trace**: This event describes the highest-level state changes of PoseidonOS.
- **Info**: This event is a normal, informative, high-level state change of PoseidonOS.
- **Warning**: This event indicates that input from the user is errorneous, or the system is not working properly.
- **Error**: This event indicates that an operation has failed because of an internal error.
- **Critical**: this event indicates a severe failure; when this event occurs, the system may not be available.

### JSON form (when structured logging is on)
```json
{"datetime":"2022-03-04 15:54:01.407056341","process_id":19646,"thread_id":19985,"pos_id":80861475,"event_id":1207,"level":"info","description":{"event_name:":"CLI_CLIENT_DISCONNECTED","message":"A client has been disconnected.","cause":"","solution":"","variables":"fd:655"},"source":"cli_server.cpp","line":"168","function":"RemoveClient","pos_version":"v0.10.6"},
```

# Log Message
Log message is the body of an event log. It describes which and why an event occurs. For a better readability of PoseidonOS event log, we introduce a guideline to log message format as follows.

PoseidonOS event log message consists of three field: ***message***, and ***cause***, ***solution***, and ***variable***. ***message*** field describes what the event is, ***cause*** field describes why it occured, ***solution*** field describes how it can be resolved. Note that only erroneous events will have values in ***cause*** and ***solution*** fields. **We strongly recommend to clearly write those fields in a sentence form (Subject + Verb + Object) as possible as you can. A sentence must start with the upper case and end with a period.** Additional values that support log message should be recorded as **variables** field. An examples of event log message are as follows: 
- **Example 1**
  - "message": "PoseidonOS failed to create an array." 
  - "cause": "The specified array name contains a special character."
  - "solution": "Remove special characters from the array name."
  - "variables": [{"array_name":"arr@y1"}]
- **Example 2**
  - "message": "Volume has been created."
  - "cause": ""
  - "solution": ""
  - "variables": [{"volume_name":"volume1","size":1024MB,"array_name":"array1"}]
- **Example 3 (Bad example)**
  - "message": "failed to create an array" - **The sentence does not start with lower case neither end with period.**
  - "cause": ""
  - "solution": ""
  - "variables": [{"volume_name":"volume1","size":1024MB,"array_name":"array1"}]

# Log File
PoseidonOS event log is recorded to /var/log/pos/pos.log in default.

- ***Warning: we recommend to record different logs to separate files. For example, record PosedionOS event log to pos.log and SPDK log to spdk.log.*** 

# Logging Guideline for Developers
You can add an event logging to PoseidonOS as follows:

## Define Your Event
Define your event in src/event/pos_event.yaml.

**Note 1**: please be aware that the event's ID and name are unique; they will be the key of an event).

**Note 2**: please be aware of the ID range. Each event category has an event ID range. Please refer to the annotations in pos_event.yaml (e.g., # Rebuild: 2800 - 2899).

**IMPORTANT**: execute YOUR_POSEIONDOS_DIR$ make install after updating pos_event.yaml. The command will install the pos_event.yaml to allow PoseidonOS to load the file at the start.

```
-
  Id: 2603
  Name: AUTO_CREATE_ARRAY_DEBUG
  Severity:
  Message:
  Cause:
  Solution:
-
  Id: 2604
  Name: YOUR_OWN_EVENT_NAME
  Severity:
  Message: YOUR_OWN_EVENT_MESSAGE
  Cause: FILL_IN_THIS_FIELD_IF_NECESSARY
  Solution: FILL_IN_THIS_FIELD_IF_NECESSARY
-
# Rebuild: 2800 - 2899
  Id: 2800
  Name: REBUILD_REQUEST
  Severity:
  Message:
  Cause:
  Solution:
-
  Id: 2801
  Name: REBUILD_DONE
  Severity:
  Message:
  Cause:
  Solution:
-
```
  ### Event Naming Guideline
  Good event names make your event logs easy-to-read and intuitive. Here are some guides for good event names.

  #### Convention
  In PoseidonOS, we set the naming convention of an event as follows:
  ```
  CATEGORY_EVENT_CAUSE
  ```
  Good event names are clear; they speak about what happened and why it happened. For example, ARRAY_DEBUG_EVENT might not be a good name because it is difficult for developers to understand at a glance what exactly occurred. So if you have to look into the source code to understand the meaning of the event name, it might not be a good name.

  Instead, ARRAY_CREATION_FAILURE_WRONGNAME, VOLUME_MOUNT_SUCCESS, and MFS_IO_FAILED_DUE_TO_ENQUEUE_FAILED can be good names (we admit both nouns and verbs in the event name). Be specific as possible!

  ### How to Add Your Event Log
  When starting PoseidonOS, the event you added in the previous step will be loaded to a map in PoseidonOS. Now, you can add your event to the source code as in the following code:
  ```
  int
ArrayManager::Create(string name, DeviceSet<string> devs, string metaFt, string dataFt)
{
    pthread_rwlock_wrlock(&arrayListLock);
    if (_FindArray(name) != nullptr)
    {
        int event = EID(YOUR_OWN_EVENT_NAME);
        POS_TRACE_WARN(event, "your_related_variable1:{}, your_related_variable2:{}", your_related_variable1, your_related_variable2);
        pthread_rwlock_unlock(&arrayListLock);
        return event;
    }
  ```
  EID() returns the event ID in the source code, and POS_TRACE_WARN() records the event logs. **You only need to specify the ID of the event and the related variables (a comma-separated key:value list) to the event.** The PoseidonOS will record the event's message, cause, and solution from the table to the log record file. The related variables will be recorded in the variable fields.

  There are some macros for event logging for different event levels and purposes. Mostly, POS_TRACE_LEVEL() macros are used. 
  ```
  #define POS_TRACE_DEBUG_IN_MEMORY(dumpmodule, eventid, ...) \
  #define POS_TRACE_INFO_IN_MEMORY(dumpmodule, eventid, ...) \
  #define POS_TRACE_TRACE_IN_MEMORY(dumpmodule, eventid, ...) \
  #define POS_TRACE_WARN_IN_MEMORY(dumpmodule, eventid, ...) \
  #define POS_TRACE_ERROR_IN_MEMORY(dumpmodule, eventid, ...) \
  #define POS_TRACE_CRITICAL_IN_MEMORY(dumpmodule, eventid, ...) \

  #define POS_TRACE_DEBUG(eventid, ...) \
  #define POS_TRACE_INFO(eventid, ...) \
  #define POS_TRACE_TRACE(eventid, ...) \
  #define POS_TRACE_WARN(eventid, ...) \
  #define POS_TRACE_ERROR(eventid, ...) \
  #define POS_TRACE_CRITICAL(eventid, ...) \

  #define POS_REPORT_TRACE
  #define POS_REPORT_WARN
  #define POS_REPORT_ERROR
  #define POS_REPORT_CRITICAL                                                                     \
  ```

  ### Log Level Guideline
  It is always challenging to choose the right event level. Here's a simple guideline for developers:
  - **Critical**: use this level when it is expected that PoseidonOS must be halted when this event occurs.
  - **Error**: use this level when it is expected that PoseidonOS can proceed but goes to an erroneous state when this event occurs.
  - **Warn**: use this level when it is expected that PoseidonOS can run normally but this event needs to be reported carefully.
  - **Trace**: use this level for the highest-level events of PoseidonOS (e.g., array creation, volume mount, CLI command receive, I/O receive, and I/O send).
  - **Info**: use this level for the high-level events of PoseidonOS.
  - **Debug**: use this level for the low-level events of PoseidonOS (e.g., loading a segment context from a file, RocksDBMetaFsIntf internal write success, and the journal log group is full). Please note that it is acceptable that the event logs with debug level affect the performance. If necessary, PoseidonOS can turn off recording those events.

## Event Log Output
The following is an example of /var/log/pos.log, where PoseidonOS event logs are recorded. As you can see, the names of the events are emphasized. The philosophy of the PoseidonOS event logging is to display what happened to the system at a glance.
```
[2022-10-19 19:09:02.029184126][15539][15579][87149155][1201][ info  ]  CLI_SERVER_INITIALIZED - The CLI server has been initialized successfully. (cause: NONE, solution: NONE, variables: max_cli_count:5), source: cli_server.cpp:424 CLIServer(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:02.029793722][15539][15578][87149155][3611][ info  ]  RESOURCE_CHECKER_EXECUTE_IN - Periodic execution of host side resource check has begun. (cause: NONE, solution: NONE, variables: running_count:0), source: resource_checker.cpp:160 Execute(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:02.039214265][15539][15578][87149155][9515][ info  ]  TELEMETRY_PUBLISHER_PUBLICATION_LIST_LOAD_SUCCESS - Succeeded to load a publication list for telemetry metrics! (cause: NONE, solution: NONE, variables: filePath:/etc/pos/publication_list_default.yaml, list_size:37), source: /home/lee/Workspace/pos/pos-release/poseidonos/src/telemetry/telemetry_client/telemetry_publisher.cpp:228 _LoadPublicationList(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:02.481061439][15539][15583][87149155][1208][ trace ]  CLI_MSG_RECEIVED - CLI server has recieved a message from a CLI client. (cause: NONE, solution: NONE, variables: request: command: "SYSTEMINFO" rid: "0ddd0bbb-4f96-11ed-9bf1-b42e99ff989b" requestor: "cli"), source: grpc_cli_server.cpp:728 _LogCliRequest(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:02.574903090][15539][15583][87149155][1209][ trace ]  CLI_MSG_SENT - CLI server has sent a message to a client. (cause: NONE, solution: NONE, variables: response: command: "SYSTEMINFO" rid: "0ddd0bbb-4f96-11ed-9bf1-b42e99ff989b" result { status { code: 0 event_name: "SUCCESS" description: "NONE" cause: "NONE" solution: "NONE" } data { version: "v0.12.0-rc0" biosVersion: "F8" biosVendor: "American Megatrends Inc." biosReleaseDate: "10/15/2019" systemManufacturer: "Gigabyte Technology Co., Ltd." systemProductName: "Z390 DESIGNARE" systemSerialNumber: "Default string" systemUuid: "032E02B4-0499-05FF-9806-9B0700080009" baseboardManufacturer: "Gigabyte Technology Co., Ltd." baseboardProductName: "Z390 DESIGNARE-CF" baseboardSerialNumber: "Default string" baseboardVersion: "x.x" processorManufacturer: "Intel(R) Corporation" processorVersion: "Intel(R) Core(TM) i9-9900K CPU @ 3.60GHz" processorFrequency: "4653 MHz" } } info { version: "v0.12.0-rc0" }, gRPC_error_code: 0, gRPC_error_details: , gRPC_error_essage: ), source: grpc_cli_server.cpp:735 _LogCliResponse(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:20.253586649][15539][15585][87149155][1208][ trace ]  CLI_MSG_RECEIVED - CLI server has recieved a message from a CLI client. (cause: NONE, solution: NONE, variables: request: command: "SETTELEMETRYPROPERTY" rid: "190fbd79-4f96-11ed-a021-b42e99ff989b" requestor: "cli" param { publicationListPath: "/etc/l" }), source: grpc_cli_server.cpp:728 _LogCliRequest(), pos_version: v0.12.0-rc0
[2022-10-19 19:09:20.253662446][15539][15585][87149155][1209][ trace ]  CLI_MSG_SENT - CLI server has sent a message to a client. (cause: NONE, solution: NONE, variables: response: command: "SETTELEMETRYPROPERTY" rid: "190fbd79-4f96-11ed-a021-b42e99ff989b" result { status { code: 9528 event_name: "TELEMETRY_SET_PROPERTY_FAILURE_INVALID_FILE" description: "Failed to set telemetry property." cause: "Could not open the file." solution: "Check if the file path is correct, or you have the permission." } } info { version: "v0.12.0-rc0" }, gRPC_error_code: 0, gRPC_error_details: , gRPC_error_essage: ), source: grpc_cli_server.cpp:735 _LogCliResponse(), pos_version: v0.12.0-rc0
```