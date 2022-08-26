## poseidonos-cli device create

Create a buffer device.

### Synopsis


Create a buffer device.

Syntax:
	poseidonos-cli device create (--device-name | -d) DeviceName --num-blocks Number --block-size BlockSize --device-type uram --numa Number
          

```
poseidonos-cli device create [flags]
```

### Options

```
  -s, --block-size uint32    The block size of the buffer device. (default 512)
  -d, --device-name string   The name of the buffer device to create.
  -t, --device-type string   The type of the buffer device to create. (default "uram")
  -h, --help                 help for create
  -b, --num-blocks uint32    The number of blocks of the buffer device. (default 8388608)
  -n, --numa uint32          The NUMA node of the buffer device.
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

* [poseidonos-cli device](poseidonos-cli_device.md)	 - Device commands for PoseidonOS.

