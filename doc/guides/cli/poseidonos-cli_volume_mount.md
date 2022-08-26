## poseidonos-cli volume mount

Mount a volume to the host.

### Synopsis


Mount a volume to the host. 

Syntax:
	mount (--volume-name | -v) VolumeName (--array-name | -a) ArrayName
	[(--subnqn | -q) TargetNVMSubsystemNVMeQualifiedName] [(--trtype | -t) TransportType]
	[(--traddr | -i) TargetAddress] [(--trsvcid | -p) TransportServiceId]

Example: 
	poseidonos-cli volume mount --volume-name Volume0 --array-name Volume0
	
         

```
poseidonos-cli volume mount [flags]
```

### Options

```
  -a, --array-name string             The name of the array where the volume belongs to.
      --force                         Force to mount this volume.
  -h, --help                          help for mount
  -q, --subnqn string                 NVMe qualified name of target NVM subsystem. When this flag is specified,
                                      		POS will check if the specified NVM subsystem exists. If it exists, 
                                      		POS will mount this volume to it. Otherwise, POS will create a new
                                      		NVM subsystem and mount this volume to it.
  -i, --target-address string         NVMe-oF target address (ex. 127.0.0.1)
  -p, --transport-service-id string   NVMe-oF transport service id (ex. 1158)
  -t, --transport-type string         NVMe-oF transport type (ex. tcp)
  -v, --volume-name string            The name of the volume to mount.
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

* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.

