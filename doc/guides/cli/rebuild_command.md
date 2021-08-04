# Rebuild Command

### REBUILDPERFIMPACT
```
$ cli rebuild perf_impact --level low
```

* It adjusts the priority of RAID rebuild I/O. The possible value is high, medium, or low.•If it is set to high, POS minimizes the time to rebuild the array, which may impact on user I/O.
* If it is set to low, POS favors user I/O over rebuild I/O, which may lead to longer rebuild time. 

* RPC request: {"rid":"fromCLI", "command":"REBUILDPERFIMPACT", "param": {"level":"low"}}
* RPC response: {"command":"REBUILDPERFIMPACT", "rid":"fromCLI", "result":{"status": {"code":0, "description":"perf_impact requested is applied" }}}
