## poseidonos-cli subsystem set-listener-ana-state

Set a listener's ana state to an NVMe-oF subsystem

### Synopsis


Set a listener's ana state to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem set-listener-ana-state (--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId (--anastate | -a) AnaState

Example:
	poseidonos-cli subsystem set-listener-ana-state -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.100.2.14 -p 1158 -a inaccessible

    

```
poseidonos-cli subsystem set-listener-ana-state [flags]
```

### Options

```
  -a, --anastate string   NVMe-oF subsytem-listener's ana state: e.g., an ANA state
  -h, --help              help for set-listener-ana-state
  -q, --subnqn string     The NQN of the subsystem to set listener's ana state.
  -i, --traddr string     NVMe-oF target address: e.g., an ip address
  -p, --trsvcid string    NVMe-oF transport service id: e.g., a port number
  -t, --trtype string     NVMe-oF transport type: e.g., tcp
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
      --timeout uint32   Timeout for this command in seconds. (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.

