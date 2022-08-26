## poseidonos-cli system set-property

Set the property of PoseidonOS.

### Synopsis


Set the property of PoseidonOS.
(Note: this command is not officially supported yet.
 It might be  possible this command cause an error.)

Syntax:
	poseidonos-cli system set-property [--rebuild-impact (highest | medium | lowest)]

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact lowest
          

```
poseidonos-cli system set-property [flags]
```

### Options

```
  -h, --help                    help for set-property
      --rebuild-impact string   The impact of rebuilding process on the I/O performance.
                                With high rebuilding-impact, the rebuilding process may
                                interfere with I/O operations more. Therefore, I/O operations may
                                slow down although rebuilding process becomes accelerated. 
                                
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

* [poseidonos-cli system](poseidonos-cli_system.md)	 - System commands for PoseidonOS.

