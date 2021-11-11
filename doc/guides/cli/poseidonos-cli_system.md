## poseidonos-cli system

System commands for PoseidonOS.

### Synopsis


System commands for PoseidonOS. Use this command category to start/stop
PoseidonOS or get the information of PoseidonOS.

Syntax: 
  poseidonos-cli system [start|stop|info|set-property] [flags]

Example (to start PoseidonOS):
  poseidonos-cli system start
	  

```
poseidonos-cli system [flags]
```

### Options

```
  -h, --help   help for system
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

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS [version 0.7]
* [poseidonos-cli system info](poseidonos-cli_system_info.md)	 - Display information of PoseidonOS.
* [poseidonos-cli system set-property](poseidonos-cli_system_set-property.md)	 - Set the property of PoseidonOS.
* [poseidonos-cli system start](poseidonos-cli_system_start.md)	 - Start PoseidonOS.
* [poseidonos-cli system stop](poseidonos-cli_system_stop.md)	 - Stop PoseidonOS.

