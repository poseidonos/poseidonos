## poseidonos-cli devel

Commands for PoseidonOS developers.

### Synopsis


Commands for PoseidonOS Developers. This command category will affect
the system seriously. Therefore, this command category must be carefully used
by developers. 

Syntax: 
  poseidonos-cli devel [resetmbr|reset-event-wrr|stop-rebuilding|update--event-wrr]

	  

```
poseidonos-cli devel [flags]
```

### Options

```
  -h, --help   help for devel
```

### Options inherited from parent commands

```
      --debug            Print response for debug.
      --fs string        Field separator for the output. (default "|")
      --ip string        Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req         Print request in JSON form.
      --json-res         Print response in JSON form.
      --node string      Name of the node to send this command. When both --ip and this flag are specified, this flag is applied only.
      --port string      Set the port number to PoseidonOS for this command. (default "18716")
      --timeout uint32   Timeout for this command in seconds. (Note: array unmount command has 30 minutes timeout.) (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS
* [poseidonos-cli devel reset-event-wrr](poseidonos-cli_devel_reset-event-wrr.md)	 - Reset the wieghts for backend events such as Flush, Rebuild, and GC to the default values.
* [poseidonos-cli devel resetmbr](poseidonos-cli_devel_resetmbr.md)	 - Reset MBR information of PoseidonOS.
* [poseidonos-cli devel stop-rebuilding](poseidonos-cli_devel_stop-rebuilding.md)	 - Stop rebulding.
* [poseidonos-cli devel update-event-wrr](poseidonos-cli_devel_update-event-wrr.md)	 - Set the weights for backend events such as Flush, Rebuild, and GC.

