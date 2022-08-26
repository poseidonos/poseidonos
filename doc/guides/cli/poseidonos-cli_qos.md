## poseidonos-cli qos

QoS commands for PoseidonOS.

### Synopsis


QoS commands for PoseidonOS. Use this command category when you want
to specify or display the quality-of-service of a specific volume. 

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
* [poseidonos-cli qos create](poseidonos-cli_qos_create.md)	 - Create qos policy for a volume(s) of PoseidonOS.
* [poseidonos-cli qos list](poseidonos-cli_qos_list.md)	 - List QoS policy for a volume(s) of PoseidonOS.
* [poseidonos-cli qos reset](poseidonos-cli_qos_reset.md)	 - Reset QoS policy for a volume(s) of PoseidonOS.

