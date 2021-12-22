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

# PoseidonOS EventID
PoseidonOS **EventID** is the unique identifier of an event. Its format is defined as follows: 

| Category | Category | Level | Reserved | Reserved | Reserved | Event Number | Event Number | Event Number |
|----------|----------|-------|----------|----------|----------|--------------|--------------|--------------|

Each field (column) in the table is represented as a digit. Thus, **EventID** of PoseidonOS is a 9-digit number.

1. **Category**: this field indicates the category of this event. Categories can be array, volume, meta, I/O, and so on.
2. **Level**: this field indicates the severity level of this event. PoseidonOS uses the same level values of [syslog](https://datatracker.ietf.org/doc/html/rfc3164) for compatibility. 
    1. **Debug (7)**: this event is logged for debugging purpose. 
    2. **Informational (6)**: this event is logged to examine that PoseidonOS is working as expected.
    3. **Warning (4)**: this event can cause a possible problem. 
    4. **Error (3)**: this event incurs an error, but PoseidonOS can continue to work. 
    5. **Critical (2)**: this event incurs a critical failure that causes a termination of PoseidonOS. 
3. **Reserved**: reserved fields for future use.
4. **Event Number**: the unique identification number for the event in the category. 

- **Example**: EventID 032000034
  - Category: allocator (03)
  - Level: Error (3)
  - Event Number: Journal Checkpoint Completed (034)

- ***Warning: please use a proper level for the event. For example, if you apply Info level to events that should have Debug level, the log can contain too much detail; the readability will be worse.***

# PoseidonOS Log Entry
PoseidonOS event log is designed for structured logging, which structurally formats the log message. The structured log messages are used as inputs to other systems for search, monitoring, analysis, security checks, and so on.

A formal description of PoseidonOS event log entry format is defined in [PoseidonOS Event Log Entry JSON Schema](log_entry_schema.json) (in JSON schema form).

**Example**
```json
{"instanceId": 1638929401,"eventId": 10005328,"datetime": "2021-12-09 03:00:93.999","level": "info","eventName":"ARRAY_CREATE_FAILURE","moduleName": "ArrayManager","errorCode": 301,"message": "Failed to create an array","cause": "Array name has a special character","trace": "","variables": [ {"arrayName": "array1"},{"volumeName": "volum@1"}],"source": "array_name_policy.cpp:80","posVersion":"pos-v0.10.0"}
```

# Log Message
Log message is the body of an event log. It describes which and why an event occurs. For a better readability of PoseidonOS event log, we introduce a guideline to log message format as follows.

PoseidonOS event log message consists of two field: ***message*** and ***cause***. ***message*** field describes what the event is, and ***cause*** field describes why it occured. **We strongly recommend to clearly write log message in a sentence form (Subject + Verb + Object) as possible as you can**. Additional values that support log message should be recorded as **variables** field. An examples of event log message are as follows: 
- **Example 1**
  - "message": "PoseidonOS failed to create an array." 
  - "cause": "The specified array name contains a special character."
  - "variables": [{"array_name":"arr@y1"}]
- **Example 2**
  - "message": "Volume has been created."
  - "cause": ""
  - "variables": [{"volume_name":"volume1","size":1024MB,"array_name":"array1"}]

# Log File
PoseidonOS event log is recorded to /var/log/pos/pos.log in default.

- ***Warning: we recommend to record different logs to separate files. For example, record PosedionOS event log to pos.log and SPDK log to spdk.log.*** 