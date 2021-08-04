### SCANDEVICE
```
$ cli device scan
```

* It scans PCI-attached NVMe devices in the system.
* RPC request: {"command":"SCANDEVICE","rid":"fromfakeclient"}
* RPC response: {"command":"SCANDEVICE","rid":"fromfakeclient","result":{"status":{"code":0,"description":"Scan Device Done"}}}

### LISTDEVICE
```
$ cli device list
```

* It shows all devices known to POS. SCANDEVICE needs to run prior to LISTDEVICE for a desired result.
* RPC request: {"command":"LISTDEVICE","rid":"fromfakeclient"}
* RPC response: {"command":"LISTDEVICE", "rid":"fromCLI", "result":{"status":{"code":0,"description":"DONE"}, "data":{"devicelist":[{"name":"samsung-unvmens-0","size":16777216,"mn":"VMware Virtual NVMe Disk","sn":"VMWare NVME-0002" ,"type":"SSD", "addr":"0000:04:00.0"}, {"name":"samsung-unvmens-1", "size":16777216, "mn":"VMware Virtual NVMe Disk", "sn":"VMWare NVME-0003", "type":"SSD", "addr":"0000:0c:00.0"}, {"name":"samsung-unvmens-2", "size":16777216, "mn":"VMware Virtual NVMe Disk", "sn":"VMWare NVME-0000" ,"type":"SSD", "addr":"0000:13:00.0"}, {"name":"samsung-unvmens-3", "size":16777216, "mn":"VMware Virtual NVMe Disk", "sn":"VMWare NVME-0001", "type":"SSD", "addr":"0000:1b:00.0"}, {"name":"uram0", "size":262144, "mn":"uram0", "sn":"uram0", "type":"NVRAM", "addr":""}]}}}
