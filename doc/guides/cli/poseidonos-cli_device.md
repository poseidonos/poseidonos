## poseidonos-cli device

Device commands for PoseidonOS.

### Synopsis


Device commands for PoseidonOS. Use this command category to create,
delete, and display devices. 

Syntax: 
  poseidonos-cli device [create|scan|list|smart] [flags]

Example (to scan devices in the system):
  poseidonos-cli device scan
	  

```
poseidonos-cli device [flags]
```

### Options

```
  -h, --help   help for device
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
* [poseidonos-cli device create](poseidonos-cli_device_create.md)	 - Create a buffer device.
* [poseidonos-cli device list](poseidonos-cli_device_list.md)	 - List all devices in the system.
* [poseidonos-cli device scan](poseidonos-cli_device_scan.md)	 - Scan devices in the system.
* [poseidonos-cli device smart-log](poseidonos-cli_device_smart-log.md)	 - Display SMART log information of a device.

