## poseidonos-cli telemetry

Telemetry commands for PoseidonOS.

### Synopsis


Telemetry commands for PoseidonOS. Use this command category to
start/stop or configure the telemetry of PoseidonOS. For example,
PoseidonOS will not gather the internal statistics once you execute
the temeletry stop command.  

Syntax: 
  poseidonos-cli telemetry [start|stop]

Example (to start telemetry PoseidonOS):
  poseidonos-cli telemetry start
	  

```
poseidonos-cli telemetry [flags]
```

### Options

```
  -h, --help   help for telemetry
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS
* [poseidonos-cli telemetry start](poseidonos-cli_telemetry_start.md)	 - Start the collection of telemetry data in PoseidonOS.
* [poseidonos-cli telemetry stop](poseidonos-cli_telemetry_stop.md)	 - Stop the collection of telemetry data in PoseidonOS.

