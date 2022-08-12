# PoseidonOS developer cheatsheet

This document describes tips for PoseidonOS developers.

## Logging an event

[PoseidonOS Event Log Management](./design/log_management.md)

You can record your custom event log to the PoseidonOS event log file (/var/log/pos/pos.log).

1. Adding a custom event

    Add your event to POS_EVENT_ID at $YOUR_POS_PATH/src/include/pos_event_id.h. Add your event ID within the proper range (e.g., CLI events use 1500-1599).
    ```
    // ------------------- CLI Server (1500 - 1599) -------------------
    CLI_EVENT_ID_START = 1500,
    CLI_SERVER_INITIALIZED,
    CLI_CLIENT_ACCEPTED,
    CLI_CLIENT_DISCONNECTED,
    CLI_YOUR_CUSTOM_EVENT_ID, // Be aware of the prefix.
    CLI_MSG_RECEIVED,
    CLI_MSG_SENT,
    
    ...
    
    CLI_EVENT_ID_END = 1599,
    ```
2. Define the event information

    Next, you should define the information (i.e., name, message, cause, and solution). In the pos_event_id.h file, you can find PosEventInfo. Add the information for your event. You don't have to record the cause and solution fields for non-erroneous events. We recommend to use the same event name as the name of the eventID variable. **We strongly recommend to clearly write those fields in a sentence form (Subject + Verb + Object) as possible as you can.**
    ```
    {EID(CLI_SERVER_INITIALIZED),
            new PosEventInfoEntry("CLI_SERVER_INITIALIZED",
                "The CLI server has been initialized successfully.", "", "")},
    {EID(CLI_CLIENT_ACCEPTED),
        new PosEventInfoEntry("CLI_CLIENT_ACCEPTED",
            "A new client has been accepted (connected).", "", "")},
    
    ...

    // Clearly write those fields in a sentence form (Subject + Verb + Object) as possible.
    {EID(CLI_YOUR_CUSTOM_EVENT_ID),
        new PosEventInfoEntry("CLI_YOUR_CUSTOM_EVENT_ID",
            "Your custom message is here.", "Your custom cause is here.", "Your solution message is here.")},
    ```
3. Log your event using macro

    Log your custom event using a macro PoseidonOS provides. You can use the following macros defined in $YOUR_POS_PATH/src/logger/logger.h according to the log levels:
    ```
    #define POS_TRACE_DEBUG(eventid, ...)
    #define POS_TRACE_INFO(eventid, ...)
    #define POS_TRACE_WARN(eventid, ...)
    #define POS_TRACE_ERROR(eventid, ...)
    #define POS_TRACE_CRITICAL(eventid, ...)
    ```
    For example, when the CLI server succeeds to send a message to the CLI client, it records an event log as below.
    ```
    if (ret < 0)
    {
        // you can input the varaibles related to the event.
        int event = EID(CLI_MSG_SENT);
        POS_TRACE_INFO(event, "fd:{}, length:{}, message:{}", client->sockfd, ret, msg);
    }
    ```
    The event log will be recorded and displayed as below.
    ```
    ...
    [10262307][21743][21815][2022-02-18 13:46:57.245728][1505][I]   CLI_MSG_SENT - MSG:"CLI server has sent a message to a client." CAUSE:"" SOLUTION:"" VARIABLES:"fd:655, length:215, message:{"command":"LISTARRAY","rid":"cd5a1bbc-9075-11ec-af45-b42e99ff989b","result":{"status":{"code":5525,"eventName":"","description":"List array failed","cause":"","solution":""},"data":{}},"info":{"version":"v0.10.6"}}" @ cli_server.cpp:127 SendMsg() - POS: v0.10.6
    [10262307][21743][21815][2022-02-18 13:46:57.245744][1503][I]   CLI_CLIENT_DISCONNECTED - MSG:"A client has been disconnected." CAUSE:"" SOLUTION:"" VARIABLES:"fd:655" @ cli_server.cpp:170 RemoveClient() - POS: v0.10.6
    ...
    ```

# Factory Reset
If you're having trouble with your PoseidonOS configuration or unknown issue,
you can reset your PoseidonOS to the initial state.

Factory Reset will
* Terminate PoseidonOS process
* Reset config file  
PoseidonOS config files exist in /etc/pos/pos.conf
* Clear log files  
PoseidonOS log files exist in /var/log/pos/*
* Reset MBR area of devices  
Write zero to MBR area
Refer to [MBR Reset](#mbr-reset) for further details
* Reset udev rule file  
Refer to [learning hotplug](../guides/getting_started/learning_hotplug.md) for further details
* Reset device driver  
Unbind and bind device to userspace driver 'uio_pci_generic'  
Same procedure with following script. `$POS_HOME/script/setup_env.sh`

Usage :
``` 
root@R2U14-PSD-3:/poseidonos# cd ./script
root@R2U14-PSD-3:/poseidonos/script# ./factory_reset.sh
```

# MBR Reset
In PoseidonOS, MBR is the first 256KB area of each device containing PoseidonOS and its array information.
Resetting MBR procedure contains resetting device drivers which unbinds all devices from uio_pci_generic driver and binds to nvme driver.
After writing zero to the MBR area of every device, it binds to uio_pci_generic driver.
And also it reset udev rule.

Usage :
``` 
cd $POS_HOME/script; ./mbr_reset.sh
```

# Configuration Reset
PoseidonOS has a default config file for general usage. You can copy and paste a default config file to reset config file.
``` bash
cp /etc/pos/default_pos.conf /etc/pos/pos.conf
# if default_pos.conf doesn't exist
cp $POS_HOME/config/pos.conf /etc/pos/pos.conf
```

# Udev Rule Setting
Udev is the dynamic device manager for the Linux kernel. Udev manages device uevents from kernel. Udev works by the rule file on default rules directory `/lib/udev/rules.d/` and custom rules directory `/etc/udev/rules.d/`. When any new nvme device attaches to the system, it binds to kernel space device driver `nvme`. Because PoseidonOS uses a userspace device driver, it needs to be changed to userspace driver `uio_pci_generic` to provide hotplug feature.  
Refer to [learning hotplug](../guides/getting_started/learning_hotplug.md) to enable hotplug by setting udev rule.

# Telemetry 
In order for POS-Telemetry to collect data, the telemetry must be started through cli and exporter must be performed.
Users can access Telemetry Metric collected through the web environment.

1. Telemetry Enable using poseidon-cli, Start pos-exporter

Usage :
``` bash
$POS_HOME/bin/poseidonos-cli telemetry start
$POS_HOME/bin/pos-exporter

2. Get Metric

$ curl <pos-ip>:2112