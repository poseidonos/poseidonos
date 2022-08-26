## poseidonos-cli subsystem add-listener

Add a listener to an NVMe-oF subsystem

### Synopsis


Add a listener to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem add-listener (--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId

Example:
	poseidonos-cli subsystem add-listener -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.100.2.14 -p 1158

    

```
poseidonos-cli subsystem add-listener [flags]
```

### Options

```
  -h, --help             help for add-listener
  -q, --subnqn string    The NQN of the subsystem to add listener.
  -i, --traddr string    NVMe-oF target address: e.g., an ip address
  -p, --trsvcid string   NVMe-oF transport service id: e.g., a port number
  -t, --trtype string    NVMe-oF transport type: e.g., tcp
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

