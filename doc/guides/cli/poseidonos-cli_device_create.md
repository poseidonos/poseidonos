## poseidonos-cli device create

Create a buffer device.

### Synopsis


Create a buffer device.

Syntax:
	poseidonos-cli device create (--device-name | -d) DeviceName --num-blocks NumBlocks --block-size BlockSize --device-type ["uram"|"pram"] --numa NumaNode.
          

```
poseidonos-cli device create [flags]
```

### Options

```
      --block-size int       The block size of the buffer device.
  -d, --device-name string   The name of the buffer device to create.
      --device-type string   The type of the buffer device to create.
  -h, --help                 help for create
      --num-blocks int       The number of blocks of the buffer device.
      --numa int             The NUMA node of the buffer device.
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

* [poseidonos-cli device](poseidonos-cli_device.md)	 - Device commands for PoseidonOS.

