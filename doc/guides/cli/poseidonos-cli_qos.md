## poseidonos-cli qos

Qos commands for PoseidonOS.

### Synopsis

Qos commands for PoseidonOS.

Syntax: 
  poseidonos-cli qos [create|reset|list] [flags]

Example:
1. To create a qos policy for a volume
  poseidonos-cli qos create --volume-name Volume0 --array-name Array0 --maxiops 1000 --maxbw 100
2. To create a qos policy for multiple volumes
  poseidonos-cli qos create --volume-name Volume0,Volume1,Volume2 --array-name Array0 --maxiops 1000 --maxbw 100
	  

```
poseidonos-cli qos [flags]
```

### Options

```
  -h, --help   help for qos
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

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS [version 0.7]
* [poseidonos-cli qos create](poseidonos-cli_qos_create.md)	 - Create qos policy for a volume(s) of PoseidonOS.
* [poseidonos-cli qos list](poseidonos-cli_qos_list.md)	 - List qos policy for a volume(s) of PoseidonOS.
* [poseidonos-cli qos reset](poseidonos-cli_qos_reset.md)	 - Reset qos policy for a volume(s) of PoseidonOS.

