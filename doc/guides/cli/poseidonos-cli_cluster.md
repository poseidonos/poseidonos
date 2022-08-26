## poseidonos-cli cluster

Cluster commands for PoseidonOS.

### Synopsis


Cluster commands for PoseidonOS. Use this command category to manage high-availability clusters.

Note: before using this command, you need to set the database information about high-availability 
clusters (IP address, port number, username, password, and database name) as environmental variables.

The variables to set are:
  POS_HA_DB_IP_ADDRESS POS_HA_DB_NAME POS_HA_DB_USERNAME POS_HA_DB_PASSWORD POS_HA_DB_NAME

Syntax: 
  poseidonos-cli cluster [ln] [flags]

Example (to list nodes in the system):
  poseidonos-cli cluster ln
	  

```
poseidonos-cli cluster [flags]
```

### Options

```
  -h, --help   help for cluster
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
* [poseidonos-cli cluster ln](poseidonos-cli_cluster_ln.md)	 - List all nodes in the system.
* [poseidonos-cli cluster lr](poseidonos-cli_cluster_lr.md)	 - List all replications in the cluster.
* [poseidonos-cli cluster lv](poseidonos-cli_cluster_lv.md)	 - List all volumes in the cluster.
* [poseidonos-cli cluster start-rep](poseidonos-cli_cluster_start-rep.md)	 - Start replication.

