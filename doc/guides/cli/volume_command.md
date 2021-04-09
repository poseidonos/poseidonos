# Volume Commands

### CREATEVOLUME
```
$ cli volume create --name volumename --size sizeinbytes --maxiops iopsvalue --maxbw bandwidthvalue --array TargetArrayName
```
* It creates POS volume. The volume name must meet the constraints described in Volumes section. If 0 is used for QoS parameters (i.e., maxiops and maxbw), POS will not throttle with the given criteria. 
* RPC request: 
  * {"command":"CREATEVOLUME", "rid":"fromfakeclient","param":{"name":"vol01", "size":4194304,"maxiops":0, "maxbw":0, "array":"TargetArrayName"}}
  * {"command":"CREATEVOLUME", "rid":"fromfakeclient", "param" :{"name":"vol01","size":4194304,  "array":"TargetArrayName"}}
  * {"command":"CREATEVOLUME", "rid":"fromfakeclient", "param":{"name": "vol01","size":4194304, "maxiops":5000,  "maxbw":6000,  "array":"TargetArrayName"}}
* RPC response: {"command":"CREATEVOLUME","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}

### DELETEVOLUME
```
$ cli volume delete --name volumename --array TargetArrayName
```
* It deletes POS volume. The volume should be in unmounted state before the command execution.
* RPC request: {"command":"DELETEVOLUME","rid":"fromfakeclient","param":{"name":"vol01", "array":"TargetArrayName"}}
* RPC response: {"command":"DELETEVOLUME","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}


### MOUNTVOLUME
```
$ cli volume mount --name volumename --array TargetArrayName [ --subnqn TargetNVMSubsystemNVMeQualifiedName ]
```
* It mounts POS volume. The volume should be in "unmounted " state before the command execution.
* RPC request: {"command":"MOUNTVOLUME","rid":"fromfakeclient","param":{"name":"vol01", "subnqn":"TargetNVMSubsystemNVMeQualifiedName", "array":"TargetArrayName"}}
* RPC response: {"command":"MOUNTVOLUME","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}


### UNMOUNTVOLUME
```
$ cli volume unmount --name volumename --array TargetArrayName
```
* It unmounts POS volume. The volume should be in "mounted" state before the command execution.
* RPC request: {"command":"UNMOUNTVOLUME","rid":"fromfakeclient","param":{"name":"vol01", "array":"TargetArrayName"}}
* RPC response: {"command":"UNMOUNTVOLUME","rid":"fromfakeclient","result":{"status":{"code":0,"description":"DONE"}}}


### LISTVOLUME
```
$ cli volume list --array TargetArrayName
```
* It lists all POS volumes in POS array.
* RPC request: {"command":"LISTVOLUME","rid":"fromfakeclient","param":{"array":"TargetArrayName"}}
* RPC response: {"command":"LISTVOLUME","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{ "array":"TargetArrayName", "volumes":[{"name":"vol1","id":0,"total":21474836480,"remain":21474836480,"status":"Mounted","maxiops":0,"maxbw":0}]}}}


### UPDATEVOLUMEQOS
```
$ cli volume update_qos --name volumename --array TargetArrayName --maxiops iopsvalue --maxbw bandwidthvalue
```
* It updates the QoS properties of POS volume. 
* RPC request: {"command":"UPDATEVOLUMEQOS","rid":"fromCLI","param":{"array":"TargetArrayName","name":"vol01","maxiops":0,"maxbw":6000}}
* RPC response: {"command":"UPDATEVOLUMEQOS","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"}}} 


### RENAMEVOLUME
```
$ cli volume rename --name oldname --newname newname  --array TargetArrayName
```
* It updates the name of POS volume. 
* RPC request: {"command":"RENAMEVOLUME","rid":"fromCLI","param":{"array":"TargetArrayName","name":"vol01","newname":"newvol01"}}
* RPC response: {"command":"RENAMEVOLUME","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"}}} 


### GETMAXVOLUMECOUNT
```
$ cli volume get_max_cnt
```
* It retrieves the configuration value for the maximum number of volumes POS could have. 
* RPC request: {"command":"GETMAXVOLUMECOUNT","rid":"fromCLI"}
* RPC response: {"command":"GETMAXVOLUMECOUNT","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{"count":"256"}}} 


### GETHOSTNQN 
```
$ cli volume get_host_nqn --name volumeName  --array TargetArrayName
```
* It retrieves host NQN mapped to the given volume name. Please note that this command may change in the future since NQN may need to be hidden to satisfy security policies. 
* RPC request: {"command":"GETHOSTNQN","rid":"fromCLI", "param":{"array":"TargetArrayName","name":"volumeName"}}
* RPC response: {"command":"GETHOSTNQN","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{"hostnqn":"HostNQN mapped to the given Volume"}}} 
