- [Introduction](#introduction)
- [Definitions](#definitions)
- [PoseidonOS EventID](#poseidonos-eventid)
- [PoseidonOS Log Entry](#poseidonos-log-entry)
- [Log Message](#log-message)
- [Log File](#log-file)

# Introduction
This document describes a log management guideline to PoseidonOS contributors. Additonally, the guideline contains the definitions and formats of PoseidonOS event log. 

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

**Example**

Plain-text form

Format: [pos_id][process_id][thread_id][datetime][level_short] event_name - MSG:"message" CAUSE:"cause" SOLUTION:"solution" VARIABLES:"comma_seperated_variables" @ source_code:line function() - POS: pos-verson

```
[10262307][21743][21776][2022-02-18 13:47:14.993519][1502][I]   CLI_CLIENT_ACCEPTED - MSG:"A new client has been accepted (connected)." CAUSE:"" SOLUTION:"" VARIABLES:"fd:655, client_ip:127.0.0.1, client_port:48492" @ cli_server.cpp:458 CLIServer() - POS: v0.10.6
```

JSON form (when structured logging is on)
```json
{"instance_id":10262307,"processId":21743,"threadId":21776,"posVersion":"v0.10.6","datetime":"2022-02-18 13:47:16.463356","logger_name":"pos_logger","level":"info","description":{"event_name:":"CLI_CLIENT_ACCEPTED","message":"A new client has been accepted (connected).","cause":"","solution":"","variables":"fd:655, client_ip:127.0.0.1, client_port:48680"},"source":"cli_server.cpp","line":"458","function":"CLIServer"}
```

# Log Message
Log message is the body of an event log. It describes which and why an event occurs. For a better readability of PoseidonOS event log, we introduce a guideline to log message format as follows.

PoseidonOS event log message consists of three field: ***message***, and ***cause***, ***solution***, and ***variable***. ***message*** field describes what the event is, ***cause*** field describes why it occured, ***solution*** field describes how it can be resolved. Note that only erroneous events will have values in ***cause*** and ***solution*** fields. **We strongly recommend to clearly write those fields in a sentence form (Subject + Verb + Object) as possible as you can**. Additional values that support log message should be recorded as **variables** field. An examples of event log message are as follows: 
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

# Log File
PoseidonOS event log is recorded to /var/log/pos/pos.log in default.

- ***Warning: we recommend to record different logs to separate files. For example, record PosedionOS event log to pos.log and SPDK log to spdk.log.*** 