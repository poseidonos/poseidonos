## poseidonos-cli volume mount-with-subsystem

Create a subsystem and add listener automatically. Mount a volume to Host.

### Synopsis


Create a subsystem and add listener automatically and then mount a volume to Host.

Syntax:
	mount-with-subsystem (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 
	(--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId

Example: 
	poseidonos-cli volume mount-with-subsystem --volume-name vol1 --subnqn nqn.2019-04.ibof:subsystem1 
	--array-name POSArray --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
	
         

```
poseidonos-cli volume mount-with-subsystem [flags]
```

### Options

```
  -a, --array-name string             The name of the array where the volume belongs to.
  -h, --help                          help for mount-with-subsystem
  -q, --subnqn string                 NQN of the subsystem to create.
  -i, --target_address string         NVMe-oF target address (ex. 127.0.0.1)
  -p, --transport_service_id string   NVMe-oF transport service id (ex. 1158)
  -t, --transport_type string         NVMe-oF transport type (ex. tcp)
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

