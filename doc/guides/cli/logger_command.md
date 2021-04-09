# Logger Commands

### Logger Info
```
$ cli logger info
```
* It prints out the configuration of POS logger: 1) minor log path, 2) major log path, 3) log rotation size/count, 4) log level, 5) dedup config, and 6) filter configuration.
* RPC request: {"command":"LOGGERINFO", "rid":"6c62bb01-432e-11eb-8d5c-005056adb61a"}
* RPC response: {"command":"LOGGERINFO", "rid":"6c62bb01-432e-11eb-8d5c-005056adb61a", "result" : {"status" : {"code":0, "description" : "ibofs logger info"} ,"data": {"minor_log_path" :"/var/log/ibofos/ibofos.log", "major_log_path":"/var/log/ibofos/ibofos_major.log", "logfile_size_in_mb":50, "logfile_rotation_count":20, "min_allowable_log_level":"debug", "deduplication_enabled":1, "deduplication_sensitivity_in_msec":20, "filter_enabled":0}}, "info":{"state":"OFFLINE", "situation":"DEFAULT", "rebuildingProgress":"0", "capacity":0, "used":0}}

### GETLOGLEVEL
```
$ cli logger get_level
```
* It prints out the current log level.
* RPC request: {"command":"GETLOGLEVEL", "rid":"814c4ba9-432f-11eb-b119-005056adb61a"}
* RPC response: {"command":"GETLOGLEVEL", "rid":"814c4ba9-432f-11eb-b119-005056adb61a", "result" : {"status"  :{"code":0 ,"description":"current log level"} ,"data":{"level":"debug"}} ,"info":{"state":"OFFLINE" ,"situation":"DEFAULT" ,"rebuildingProgress":"0" ,"capacity":0,"used":0}}

### SETLOGLEVEL
```
$ cli logger set_level --level debug
```
* It sets the log level of POS logger. The possible values for "level" are "debug, info, trace, warning, error, critical, off".
* RPC request: {"command":"SETLOGLEVEL", "rid":"da7b25fa-4330-11eb-9ed8-005056adb61a", "param":{"level":"debug"}}
* RPC response: {"command":"SETLOGLEVEL", "rid":"da7b25fa-4330-11eb-9ed8-005056adb61a", "result":{"status":{"code":0,"description":"log level changed to debug"}}, "info":{"state": "OFFLINE","situation":"DEFAULT", "rebuildingProgress":"0", "capacity":0, "used":0}}

### APPLYLOGFILTER
```
$ cli logger apply_filter
```
* It sets a log filter based on the CPU core number to capture logs from. The filter needs to be encoded in a local file for POS to retrieve from.
The filter file path is /var/log/ibofos/filter. The file content should look like the following:
   ```
   include:0-8,10,12
   exclude:7
   ```
* RPC request: {"command":"APPLYLOGFILTER", "rid":"b0b8560f-4333-11eb-bb80-005056adb61a"}
RPC response: {"command":"APPLYLOGFILTER", "rid":"b0b8560f-4333-11eb-bb80-005056adb61a", "result":{"status" :{"code":0, "description":"filter is applied"}} ,"info":{"state":"OFFLINE", "situation":"DEFAULT","rebuildingProgress":"0", "capacity":0, "used":0}}
