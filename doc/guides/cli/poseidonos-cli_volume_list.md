## poseidonos-cli volume list

List all volumes of an array.

### Synopsis

List all volumes of an array.

Syntax:
	poseidonos-cli volume list [(--array-name | -a) ArrayName] .

Example (listing volumes from a specific array):
	poseidonos-cli volume list --array-name Array0
          

```
poseidonos-cli volume list [flags]
```

### Options

```
  -a, --array-name string   The Name of the array of volumes to list
  -h, --help                help for list
```

### Options inherited from parent commands

```
      --debug         Print response for debug
      --fs string     Field separator for the output (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command (default "127.0.0.1")
      --json-req      Print request in JSON form
      --json-res      Print response in JSON form
      --port string   Set the port number to PoseidonOS for this command (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity
```

### SEE ALSO

* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.

