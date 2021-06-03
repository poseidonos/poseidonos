# Device APIs
## LISTDEVICE
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         |                                       |
| URL          | http://{{host}}/api/ibofos/v1/devices |

### REST response - successful
```
{
    "rid": "7ec6e965-dc86-4a95-8a3a-353dc36478a1",
    "lastSuccessTime": 1588920642,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "DONE"
        },
        "data": {
            "devicelist": [
                {
                    "addr": "0000:04:00.0",
                    "class": "SYSTEM",
                    "mn": "VMware Virtual NVMe Disk",
                    "name": "unvme-ns-0",
                    "size": 16777216,
                    "sn": "VMWare NVME-0002",
                    "type": "SSD"
                },
                {
                    "addr": "0000:0c:00.0",
                    "class": "SYSTEM",
                    "mn": "VMware Virtual NVMe Disk",
                    "name": "unvme-ns-1",
                    "size": 16777216,
                    "sn": "VMWare NVME-0003",
                    "type": "SSD"
                },
                {
                    "addr": "0000:13:00.0",
                    "class": "SYSTEM",
                    "mn": "VMware Virtual NVMe Disk",
                    "name": "unvme-ns-2",
                    "size": 16777216,
                    "sn": "VMWare NVME-0000",
                    "type": "SSD"
                },
                {
                    "addr": "0000:1b:00.0",
                    "class": "SYSTEM",
                    "mn": "VMware Virtual NVMe Disk",
                    "name": "unvme-ns-3",
                    "size": 16777216,
                    "sn": "VMWare NVME-0001",
                    "type": "SSD"
                },
                {
                    "addr": "",
                    "class": "SYSTEM",
                    "mn": "uram0",
                    "name": "uram0",
                    "size": 262144,
                    "sn": "uram0",
                    "type": "NVRAM"
                }
            ]
        }
    },
    "info": {
        "state": "OFFLINE",
        "situation": "DEFAULT",
        "rebuliding_progress": 0,
        "capacity": 0,
        "used": 0
    }
}
```

## SCANDEVICE
| REST element |                      Value                     |
|:------------:|:----------------------------------------------:|
| Method       | GET                                            |
| Type         |                                                |
| URL          | http://{{host}}/api/ibofos/v1/devices/all/scan |

### REST response - successful
```
{
    "rid": "3b6f2a86-7369-40e0-9c63-65cdf417fad4",
    "lastSuccessTime": 1597819950,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        }
    },
    "info": {
        "capacity": 0,
        "rebuildingProgress": "0",
        "situation": "DEFAULT",
        "state": "OFFLINE",
        "used": 0
    }
}
```

## SMART
| REST element |                            Value                            |
|:------------:|:-----------------------------------------------------------:|
| Method       | GET                                                         |
| Type         |                                                             |
| URL          | http://{{host}}/api/ibofos/v1/devices/{{deviceName1}}/smart |

### REST response - successful
```
{
    "rid": "4ed9f174-7458-453a-a3a9-63a81b4cdc8c",
    "lastSuccessTime": 1597910447,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        },
        "data": {
            "availableSpare": "1%",
            "availableSpareSpace": "OK",
            "availableSpareThreshold": "100%",
            "contollerBusyTime": "0x50000000000000000m",
            "criticalTemperatureTime": "0m",
            "currentTemperature": "11759C",
            "dataUnitsRead": "0x60000000000000000",
            "dataUnitsWritten": "0x50000000000000000",
            "deviceReliability": "OK",
            "hostReadCommands": "0x17700000000000000000",
            "hostWriteCommands": "0x13880000000000000000",
            "lifePercentageUsed": "0%",
            "lifetimeErrorLogEntries": "0",
            "powerCycles": "0xA0000000000000000",
            "powerOnHours": "0x3C0000000000000000h",
            "readOnly": "No",
            "temperature": "OK",
            "unrecoverableMediaErrors": "0",
            "unsafeShutdowns": "0",
            "volatileMemoryBackup": "OK",
            "warningTemperatureTime": "0m"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 0
    }
}
```

