## poseidonos-cli array mount

Mount an array to PoseidonOS.

### Synopsis


Mount an array to PoseidonOS. Use this command before creating a volume.
You can create a volume from an array only when the array is mounted. 

Syntax:
	mount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array mount --array-name Array0
	
         

```
poseidonos-cli array mount [flags]
```

### Options

```
  -a, --array-name string      The name of the array to mount
  -w, --enable-write-through   When specified, the array to be mounted will work with write through mode.
  -h, --help                   help for mount
  -i, --traddr string          Default target IP address for the array.
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

