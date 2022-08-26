## poseidonos-cli array list

List arrays of PoseidonOS or display information of an array.

### Synopsis


List arrays of PoseidonOS or display information of an array.
When you specify the name of a specific array, this command will
display the detailed information about the array. Otherwise, this
command will display the brief information about all the arrays
in PoseidonOS. 

Syntax:
	poseidonos-cli array list [(--array-name | -a) ArrayName]

Example 1 (listing all arrays): 
	poseidonos-cli array list

Example 2 (listing a specific array):
	poseidonos-cli array list --array-name Array0
          

```
poseidonos-cli array list [flags]
```

### Options

```
  -a, --array-name string   The name of the array to list. If not specified, all arrays
                            		will be displayed.
  -h, --help                help for list
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

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

