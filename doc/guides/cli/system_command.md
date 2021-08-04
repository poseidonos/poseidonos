# System Commands

### EXITIBOFOS
```
$ cli system exit
```
* It shuts down POS.
* RPC request: {"command":"EXITIBOFOS","rid":"fromfakeclient"}
* RPC response: {"command":"EXITIBOFOS", "rid":"fromfakeclient", "result":{"status":{"code":0,"description":"DONE"}}}

### GETIBOFOSINFO
```
$ cli system info
```
* It shows the current state of POS.
* RPC request: {"rid":"fromCLI","command":"GETIBOFOSINFO"}
* RPC response: {"rid":"fromCLI","lastSuccessTime":1596673689,"result":{"status":{"module":"","code":0,"description":"get version"},"data":{"version":"pos-0.9.2"}} 
