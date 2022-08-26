## poseidonos-cli array addspare

Add a device as a spare to an array.

### Synopsis


Add a device as a spare to an array. Use this command when you want 
to add a spare device to an array that was created already. 
Please note that the capacity of the spare device must be equal to or greater than the smallest capacity among existing array devices.

Syntax:
	poseidonos-cli array addspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array addspare --spare nvme5 --array-name array0
          

```
poseidonos-cli array addspare [flags]
```

### Options

```
  -a, --array-name string   The name of the array to add a spare device.
  -h, --help                help for addspare
  -s, --spare string        The name of the device to be added to the specified array.
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

