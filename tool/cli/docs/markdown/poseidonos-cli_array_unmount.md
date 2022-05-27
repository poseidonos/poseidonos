## poseidonos-cli array unmount

Unmount an array from PoseidonOS.

### Synopsis


Unmount an array from PoseidonOS.

Syntax:
	unmount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array unmount --array-name Array0
          

```
poseidonos-cli array unmount [flags]
```

### Options

```
  -a, --array-name string   The name of the array to unmount.
      --force               Force to unmount this array.
  -h, --help                help for unmount
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

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

