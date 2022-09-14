## poseidonos-cli array rebuild

Instantly trigger the rebuild for an array.

### Synopsis


If there is a pending rebuild operation, immediately execute the rebuild.
Use this command when you would like to start array rebuild.

Syntax:
	rebuild (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rebuild --array-name Array0
	
         

```
poseidonos-cli array rebuild [flags]
```

### Options

```
  -a, --array-name string   The name of the array to mount
  -h, --help                help for rebuild
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

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

