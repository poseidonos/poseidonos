## poseidonos-cli devel reset-event-wrr

Reset the wieghts for backend events such as Flush, Rebuild, and GC to the default values.

### Synopsis


Reset the wieghts for backend events such as Flush, Rebuild, and GC to the default values.

Syntax:
	poseidonos-cli devel reset-event-wrr
          

```
poseidonos-cli devel reset-event-wrr [flags]
```

### Options

```
  -h, --help   help for reset-event-wrr
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

* [poseidonos-cli devel](poseidonos-cli_devel.md)	 - Commands for PoseidonOS developers.

