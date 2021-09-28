## poseidonos-cli array delete

Delete an array from PoseidonOS.

### Synopsis


Delete an array from PoseidonOS. After executing this command, 
the data and volumes in the array will be deleted too.

Syntax:
	poseidonos-cli array delete (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array delete --array-name Array0	
          

```
poseidonos-cli array delete [flags]
```

### Options

```
  -a, --array-name string   The name of the array to delete
      --force               Force to delete this array (array must be unmounted first).
  -h, --help                help for delete
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

