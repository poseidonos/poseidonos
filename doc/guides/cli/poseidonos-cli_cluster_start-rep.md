## poseidonos-cli cluster start-rep

Start replication.

### Synopsis


Start replication.

Syntax:
	poseidonos-cli cluster start-rep flags
          

```
poseidonos-cli cluster start-rep [flags]
```

### Options

```
  -h, --help                               help for start-rep
  -a, --primary-array-name string          Name of array of primary node.
  -n, --primary-node-name string           Name of primary node.
  -v, --primary-volume-name string         Name of volume of primary node.
  -w, --primary-wal-volume-name string     Name of wal volume of primary node.
  -s, --secondary-array-name string        Name of array of secondary node.
  -m, --secondary-node-name string         Name of secondary node.
  -b, --secondary-volume-name string       Name of volume of secondary node.
  -e, --secondary-wal-volume-name string   Name of wal volume of secondary node.
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

