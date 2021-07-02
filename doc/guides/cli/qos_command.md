# QoS Command

## New CLI

### CREATEQOSVOLUMEPOLICY
```
$ poseidonos-cli qos create --volume-name VolumeName (--array-name | -a) ArrayName --minbw min_bw --maxbw max_bw --miniops min_iops --maxiops max_iops
```
* It updates the QoS properties of POS volume(s).
* RPC request: {"command":"CREATEQOSVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"}],"miniops":-1,"maxiops":100,"minbw":-1,"maxbw":-1,"array":"POSArray"}}
* RPC response: {"command":"CREATEQOSVOLUMEPOLICY","rid":"fromCLI","result":{"status":{"code":0,"description":"Volume Qos Policy Create"}},"info":{"version":"pos-0.9.0"}}

### RESETQOSVOLUMEPOLICY
```
$ poseidonos-cli qos reset --volume-name VolumeName (--array-name | -a) ArrayName
```
* It resets the QoS properties of POS volume(s).
* RPC request:  {"command":"RESETQOSVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"}],"array":"POSArray"}
* RPC response: {"command":"RESETQOSVOLUMEPOLICY","rid":"fromCLI","result":{"status":{"code":0,"description":"Volume Qos Policy Reset"}},"info":{"version":"pos-0.9.0"}}

### LISTQOSPOLICIES
```
$ poseidonos-cli qos list --volume-name VolumeName (--array-name | -a) ArrayName
```
* It resets the QoS properties of POS volume(s).
* RPC request: {"command":"LISTQOSPOLICIES","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"}],"array":"POSArray"}}
* RPC response : {"command":"LISTQOSPOLICIES","rid":"fromCLI","result":{"status":{"code":0,"description":"List of Volume Policies in POSArray"},"data":{"arrayName":[{"ArrayName":"POSArray"}],"rebuildPolicy":[{"rebuild":"low"}],"volumePolicies":[{"name":"vol1","id":0,"minbw":0,"miniops":0,"maxbw":0,"maxiops":100,"min_bw_guarantee":"No","min_iops_guarantee":"No"}]}},"info":{"version":"pos-0.9.0"}}

## Old CLI (to be discontinued)

### QOSCREATEVOLUMEPOLICY
```
$ cli qos vol_policy ---vol volumename --minbw min_bw --maxbw max_bw --miniops min_iops --maxiops max_iops --array arrayname
```
* It updates the QoS properties of POS volume(s).
* RPC request: {"command":"QOSCREATEVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"},{"volumeName":"vol2"},{"volumeName":"vol3"}],"maxbw":2000,"maxiops":10000}}
* RPC response: {"Request":{"command":"QOSCREATEVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"}],"minbw":200,"miniops":100}}, "Response":{"rid":"fromCLI","lastSuccessTime":1621922375,"result":{"status":{"module":"","code":0,"description":"Volume Qos Policy Create"}},"info":{"version":"pos-0.8.2"}}}

### QOSRESETVOLUMEPOLICY
```
$ cli qos vol_reset ---vol volumename --array arrayname
```
* It resets the QoS properties of POS volume(s).
* RPC request: {"command":"QOSRESETVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"},{"volumeName":"vol2"},{"volumeName":"vol3"}]}}
* RPC response: {"Request":{"command":"QOSRESETVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"}]}},"Response":{"rid":"fromCLI","lastSuccessTime":1621922448,"result":{"status":{"module":"","code":0,"description":"Volume Qos Policy Reset"}},"info":{"version":"pos-0.8.2"}}}

### QOSLISTPOLICIES
```
$ cli qos list ---vol volumename --array arrayname
```
* It resets the QoS properties of POS volume(s).
* RPC request: {"command":"QOSLISTPOLICIES","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"},{"volumeName":"vol2"}]}}
* RPC response: {{"Request":{"command":"QOSLISTPOLICIES","rid":"fromCLI","param":{"vol":[{"volumeName":"vol1"},{"volumeName":"vol2"}]}}, "Response":{"rid":"fromCLI","lastSuccessTime":1621922297,"result":{"status":{"module":"","code":0,"description":"List of Volume Policies in POSArray"},"data":{"rebuildPolicy":[{"rebuild impact":"low"}],"volumePolicies":[{"id":0,"maxbw":0,"maxiops":0,"min bw guarantee":"No","min iops guarantee":"No","minbw":0,"miniops":0,"name":"vol1"},{"id":1,"maxbw":0,"maxiops":0,"min bw guarantee":"No","min iops guarantee":"No","minbw":0,"miniops":0,"name":"vol2"}]}},"info":{"version":"pos-0.8.2"}}}
* 
