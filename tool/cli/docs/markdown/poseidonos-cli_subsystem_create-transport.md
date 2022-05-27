## poseidonos-cli subsystem create-transport

Create NVMf transport to PoseidonOS.

### Synopsis


Create NVMf transport to PoseidonOS.

Syntax:
	poseidonos-cli subsystem create-transport (--trtype | -t) TransportType [(--buf-cache-size | -c) BufCacheSize] [--num-shared-buf NumSharedBuffers]

Example:
	poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
    

```
poseidonos-cli subsystem create-transport [flags]
```

### Options

```
  -c, --buf-cache-size int   The number of shared buffers to reserve for each poll group (default : 64).
  -h, --help                 help for create-transport
      --num-shared-buf int   The number of pooled data buffers available to the transport.
  -t, --trtype string        Transport type (ex. TCP).
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

* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.

