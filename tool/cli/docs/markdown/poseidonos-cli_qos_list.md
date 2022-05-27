## poseidonos-cli qos list

List QoS policy for a volume(s) of PoseidonOS.

### Synopsis


List QoS policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos list [(--volume-name | -v) VolumeName] [(--array-name | -a) ArrayName]

Example: 
	poseidonos-cli qos create --volume-name Volume0 --array-name Array0
          

```
poseidonos-cli qos list [flags]
```

### Options

```
  -a, --array-name string    The name of the array where the volume is created from.
  -h, --help                 help for list
  -v, --volume-name string   A comma-seperated list of volumes to set qos policy for.
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli qos](poseidonos-cli_qos.md)	 - QoS commands for PoseidonOS.

