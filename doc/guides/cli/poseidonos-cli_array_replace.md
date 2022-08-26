## poseidonos-cli array replace

Replace a data device with an available spare device in array.

### Synopsis


Replace a data device with an available spare device in array. Use this command when you expect
a possible problem of a data device. If there is no available spare device, this command will fail.

Syntax:
	poseidonos-cli array replace (--data-device | -d) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array replace --data-device nvme5 --array-name array0
          

```
poseidonos-cli array replace [flags]
```

### Options

```
  -a, --array-name string    The name of the array of the data and spare devices.
  -d, --data-device string   The name of the device to be replaced with.
  -h, --help                 help for replace
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

