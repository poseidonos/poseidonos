# Array APIs
## CREATEARRAY
| REST element |                Value                |
|:------------:|:-----------------------------------:|
| Method       | POST                                |
| Type         | RAW                                 |
| URL          | http://{{host}}/api/ibofos/v1/array |

### REST body in the request
```
{
    "param": {
        "name": "{{arrayName}}",
        "raidtype": "RAID5",
        "buffer": [
            {
                "deviceName": "uram0"
            }
        ],
        "data": [
            {
                "deviceName": "{{deviceName1}}"
            },
            {
                "deviceName": "{{deviceName2}}"
            },
            {
                "deviceName": "{{deviceName3}}"
            }
        ],
        "spare": [
            {
                "deviceName": "{{deviceName4}}"
            }
        ]
    }
}
```
### REST response - success
```
{
    "rid": "cc3eed56-3478-4180-af0b-eac6b88f264f",
    "lastSuccessTime": 1597819968,
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
### REST response - failed by even 2504
```
{
    "rid": "3cfc6d1e-6595-4aad-829a-bfca0d831069",
    "lastSuccessTime": 1597819934,
    "result": {
        "status": {
            "module": "Array",
            "code": 2504,
            "level": "ERROR",
            "description": "Device not found"
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

## DELETEARRAY
| REST element |                       Value                       |
|:------------:|:-------------------------------------------------:|
| Method       | DELETE                                            |
| Type         | RAW                                               |
| URL          | http://{{host}}/api/ibofos/v1/array/{{arrayName}} |


### REST response - success
```
{
    "rid": "f0755583-73c9-436c-9e10-c53d36418fa9",
    "lastSuccessTime": 1597910488,
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
### REST response - failed
```
{
    "rid": "6426aca5-2d99-496a-9341-7e1e962dcceb",
    "lastSuccessTime": 1597820457,
    "result": {
        "status": {
            "module": "Array",
            "code": 2500,
            "level": "ERROR",
            "description": "Array is alreday mounted."
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

## LISTARRAYDEVICE
| REST element |                           Value                           |
|:------------:|:---------------------------------------------------------:|
| Method       | GET                                                       |
| Type         | RAW                                                       |
| URL          | http://{{host}}/api/ibofos/v1/array/{{arrayName}}/devices |

### REST response - success
```
{
    "rid": "6e787e27-1964-44da-bcdf-b5f44ffbd1a3",
    "lastSuccessTime": 1588920682,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "DONE"
        },
        "data": {
            "devicelist": [
                {
                    "name": "uram0",
                    "type": "BUFFER"
                },
                {
                    "name": "unvme-ns-0",
                    "type": "DATA"
                },
                {
                    "name": "unvme-ns-1",
                    "type": "DATA"
                },
                {
                    "name": "unvme-ns-2",
                    "type": "DATA"
                },
                {
                    "name": "unvme-ns-3",
                    "type": "SPARE"
                }
            ]
        }
    },
    "info": {
        "state": "OFFLINE",
        "situation": "DEFAULT",
        "rebulidingProgress": 0,
        "capacity": 0,
        "used": 0
    }
}
```

## LOADARRAY
| REST element |                           Value                           |
|:------------:|:---------------------------------------------------------:|
| Method       | GET                                                       |
| Type         | RAW                                                       |
| URL          | http://{{host}}/api/ibofos/v1/{{arrayName}}/POSArray/load |

### REST response - failed
```
{
    "rid": "c6c54c1f-13ef-46f6-9e0c-3d1a4a1b2ee8",
    "lastSuccessTime": 1596081038,
    "result": {
        "status": {
            "module": "Array",
            "code": 2509,
            "level": "ERROR",
            "description": "MBR read error"
        }
    },
    "info": {
        "state": "OFFLINE",
        "situation": "DEFAULT",
        "rebuildingProgress": "0",
        "capacity": "0",
        "used": "0"
    }
}
```

## REMOVEDEVICE
| REST element |                                   Value                                  |
|:------------:|:------------------------------------------------------------------------:|
| Method       | DELETE                                                                   |
| Type         | RAW                                                                      |
| URL          | http://{{host}}/api/ibofos/v1/array/{{arrayName}}/devices/{{deviceName}} |

### REST response - success
```
{
    "rid": "2e7818c7-34e4-4668-9663-b5670a4678a1",
    "lastSuccessTime": 1597910417,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
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
### REST response - failed
```
{
    "rid": "6827ac78-40e2-47f6-a3b9-1a10224e694c",
    "lastSuccessTime": 1597910302,
    "result": {
        "status": {
            "module": "Array",
            "code": 2501,
            "level": "ERROR",
            "description": "Array is already umounted."
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
