# Array Command

### CREATEARRAY
```
$ cli array create -b dev1 -d dev3,dev4,dev5,dev6 -s devspare --name POSArray --raidtype RAID5
```

* It creates POS array. It accepts a list of devices to configure the array. •-b: a device to use for write buffer (dev1)
* -d: devices to use for data devices (dev3 ~ dev6)
* -s: device(s) to use for space device(s) (devspare)

* RPC request: {"command":"CREATEARRAY","rid":"fromfakeclient","param":{"name":"POSArray", "raidtype":"RAID5", "buffer":[{"deviceName":"dev1"}],"data":[{"deviceName":"dev3"},{"deviceName":"dev4"},{"deviceName":"dev5"},{"deviceName":"dev6"}],"spare":[{"deviceName":"devspare"}]}}
* RPC response: {"command":"CREATEARRAY","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}

### DELETEARRAY
```
$ cli array delete --name TargetArrayName
```

* It deletes POS array. The array state should be in "unmounted" state.
* RPC request: {"rid":"fromCLI","command":"DELETEARRAY","param":{"name":"TargetArrayName"}}
* RPC response: {"command":"DELETEARRAY","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}

### MOUNTARRAY
```
$ cli array mount --name TargetArrayName
```

* It mounts POS array. As a result, POS partitions will be created and components will be initialized.
* RPC request: {"rid":"fromCLI","command":"MOUNTARRAY","param":{"name":"TargetArrayName"}}
* RPC response: {"command":"MOUNTARRAY","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}

### UNMOUNTARRAY
```
$ cli array unmount --name TargetArrayName
``` 

* It unmounts POS array. POS volumes within the array will be unmounted and the components will be unloaded. Then, the array will be able to handle SYSTEM EXIT command. 
* RPC request: {"rid":"fromCLI","command":"UNMOUNTARRAY","param":{"name":"TargetArrayName"}}
* RPC response: {"command":"UNMOUNTARRAY","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}

### LISTARRAYDEVICE
```
$ cli array list_device --name TargetArrayName
``` 

* It shows all devices in POS array
* RPC request: {"rid":"fromCLI","command":"LISTARRAYDEVICE","param":{"name":"TargetArrayName"}}
* RPC response: {"command":"LISTARRAYDEVICE","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{"name":"TargetArrayName", "devicelist":[{"type":"BUFFER","name":"uram0"},{"type":"DATA","name":"samsung-unvmens-0"},{"type":"DATA","name":"samsung-unvmens-1"},{"type":"DATA","name":"samsung-unvmens-2"},{"type":"SPARE","name":"samsung-unvmens-3"}]}}}

### ARRAYINFO
```
$ cli array info --name TargetArrayName
```

* It shows POS array information.
* RPC request: {"rid":"fromCLI","command":"ARRAYINFO","param":{"name":"TargetArrayName"}}
* RPC response: {"command":"ARRAYINFO","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{"name":"TargetArrayName", "state": "NORMAL","devicelist":[{"type":"BUFFER","name":"uram0"},{"type":"DATA","name":"unvme-ns-0"},{"type":"DATA","name":"unvme-ns-1"},{"type":"DATA","name":"unvme-ns-2"},{"type":"SPARE","name":"unvme-ns-3"}]}}}

### ADDDEVICE
```
$ cli array add --spare dev_name --array TargetArrayName
```

* It adds a spare device to POS array. Currently, only "–spare" option is valid. 
* RPC request: {"rid":"fromCLI","command":"ADDDEVICE","param":{"array":"TargetArrayName","spare":[{"deviceName":"dev_name"}]}}
* RPC response: {"command":"ADDDEVICE","rid":"fromCLI","result":{"status":{"code":0,"description":"success"}}}

### REMOVEDEVICE
```
$ cli array remove --spare dev_name --array TargetArrayName
``` 

* It removes a spare device from POS array
* RPC request: {"rid":"fromCLI","command":"REMOVEDEVICE","param":{"array":"TargetArrayName","spare":[{"deviceName":"dev_name"}]}}
* RPC response: {"command":"REMOVEDEVICE","rid":"fromCLI","result":{"status":{"code":0,"description":"success"}}}

### LISTARRAY
```
$ cli array list
```

* It shows all POS arrays. 
* RPC request: {"rid":"fromCLI","command":"LISTARRAY","param":{}}
* RPC response: {"command":LISTARRAY","rid":"fromCLI","result":{"status":{"module":"","code":0,"description":"list of array and its devices "},"data":{"arrayList":[{array1},{array2}]}},"info":{POSINFO()}}
