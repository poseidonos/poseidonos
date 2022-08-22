## poseidonos-cli devel update-event-wrr

Set the weights for backend events such as Flush, Rebuild, and GC.

### Synopsis


Set the weights for backend events such as Flush, Rebuild, and GC.

Syntax:
	poseidonos-cli devel update-event-wrr --name ( flush | fe_rebuild | rebuild | gc ) --weight ( 1 | 2 | 3 )
          

```
poseidonos-cli devel update-event-wrr [flags]
```

### Options

```
  -h, --help          help for update-event-wrr
      --name string   Event name.
      --weight int    Weight. (default 20)
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
      --timeout uint32   Timeout for this command in seconds. (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli devel](poseidonos-cli_devel.md)	 - Commands for PoseidonOS developers.

