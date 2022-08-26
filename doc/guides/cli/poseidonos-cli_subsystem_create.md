## poseidonos-cli subsystem create

Create an NVMe-oF subsystem to PoseidonOS.

### Synopsis

Create an NVMe-oF subsystem to PoseidonOS.

Syntax:
	poseidonos-cli subsystem create (--subnqn | -q) SubsystemNQN 
	[--serial-number SerialNumber] [--model-number ModelNumber] 
	[(--max-namespaces | -m) MaxNamespace] [(--allow-any-host | -o)] [(--ana-reporting | -r)]

Example:
	poseidonos-cli subsystem create --subnqn nqn.2019-04.pos:subsystem1 
	--serial-number POS00000000000001 --model-number POS_VOLUME_EXTENSION -m 256 -o
    

```
poseidonos-cli subsystem create [flags]
```

### Options

```
  -o, --allow-any-host          Allow any host to connect (don't enforce host NQN whitelist). Default : false
  -r, --ana-reporting           Enable ANA reporting feature. Default : false
  -h, --help                    help for create
  -m, --max-namespaces uint32   Maximum number of namespaces allowed. Default : 256
      --model-number string     Model Number of the subsystem to create. Default : POS_VOLUME_EXTENTION
      --serial-number string    Serial Number of the subsystem to create. Default : POS00000000000000
  -q, --subnqn string           NQN of the subsystem to create
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

* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.

