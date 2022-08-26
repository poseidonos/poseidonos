## poseidonos-cli cluster lr

List all replications in the cluster.

### Synopsis


List all replications in the cluster.

Syntax:
	poseidonos-cli cluster lr
          

```
poseidonos-cli cluster lr [flags]
```

### Options

```
  -h, --help   help for lr
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

* [poseidonos-cli cluster](poseidonos-cli_cluster.md)	 - Cluster commands for PoseidonOS.

