# EXITIBOFOS
```
$ cli system exit
```
It shuts down POS.
RPC request: {"command":"EXITIBOFOS","rid":"fromfakeclient"}
RPC response: {"command":"EXITIBOFOS", "rid":"fromfakeclient", "result":{"status":{"code":0,"description":"DONE"}}}

# GETIBOFOSINFO
```
$ cli system info
```
It shows the current state of POS.
RPC request: {"rid":"fromCLI","command":"GETIBOFOSINFO"}
RPC response: {"command":"GETIBOFOSINFO", "rid":"fromCLI", "result": {"status":{"code":0, "description":"DONE"}, "data": {"state":"NORMAL", "situation":"NORMAL", "rebuilding_progress":0, "capacity":120795955200, "used":107374182400}}}

# GETVERSION
```
$ cli system version
```
It retrieves the version of POS.
RPC request: {"rid":"fromCLI","command":"GETVERSION"}
RPC response: {"rid":"fromCLI", "lastSuccessTime":1596673689, "result" : {"status":{"module":"" ,"code":0 ,"description":"get version"}, "data" : {"version":"pos-0.7.3"}}
