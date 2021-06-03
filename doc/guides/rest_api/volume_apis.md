# Volume APIs
## CREATEVOLUME
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | POST                                  |
| Type         | RAW                                   |
| URL          | http://{{host}}/api/ibofos/v1/volumes |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}",
        "name": "{{volumeName1}}",
        "size": 4194304,
        "maxbw": 0,
        "maxiops": 0
    }
}
```

### REST response - successful
```
{
    "rid": "bde37273-adc4-459f-883b-cf5ea2542134",
    "lastSuccessTime": 1597910684,
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
        "used": 4194304
    }
}
```
### REST response - failed
```
{
    "rid": "1ce8c5c3-d2f7-4ac8-9e59-2478605ef11d",
    "lastSuccessTime": 1597910744,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2022,
            "level": "WARN",
            "description": "Volume name is duplicated",
            "problem": "A volume with a duplicate name already exists",
            "solution": "Enter a different volume name"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 4194304
    }
}
```

## DELETEVOLUME
| REST element |                         Value                         |
|:------------:|:-----------------------------------------------------:|
| Method       | DELETE                                                |
| Type         | RAW                                                   |
| URL          | http://{{host}}/api/ibofos/v1/volumes/{{volumeName1}} |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}"
    }
}
```

### REST response - successful
```
{
    "rid": "805514bf-445b-40b6-9b84-b33a6d07e409",
    "lastSuccessTime": 1597910838,
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
        "used": 4194304
    }
}
```

### REST response - failed
```
{
    "rid": "9139d5fa-f93f-4ecb-a9f4-5fe9cc553a9d",
    "lastSuccessTime": 1597910848,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2010,
            "level": "WARN",
            "description": "The requested volume does not exist",
            "problem": "The volume with the requested volume name or volume ID does not exist",
            "solution": "Enter the correct volume name or volume ID after checking the volume list"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 4194304
    }
}
```

## LISTVOLUME
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         | RAW                                   |
| URL          | http://{{host}}/api/ibofos/v1/volumes |

### REST response - successful
```
{
    "rid": "f51223bf-db87-4308-b49b-64b981cc6d9e",
    "lastSuccessTime": 1597910789,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        },
        "data": {
            "volumes": [
                {
                    "id": 0,
                    "maxbw": 6000,
                    "maxiops": 0,
                    "name": "newvol01",
                    "remain": 4194304,
                    "status": "Unmounted",
                    "total": 4194304
                },
                {
                    "id": 1,
                    "maxbw": 0,
                    "maxiops": 0,
                    "name": "vol01",
                    "remain": 4194304,
                    "status": "Unmounted",
                    "total": 4194304
                }
            ]
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 8388608
    }
}
```

## MAXVOLUMECOUNT
| REST element |                      Value                     |
|:------------:|:----------------------------------------------:|
| Method       | GET                                            |
| Type         | RAW                                            |
| URL          | http://{{host}}/api/ibofos/v1/volumes/maxcount |

### REST response - successful
```
{
    "rid": "10dd290c-9bc2-4e32-971f-4550866fd646",
    "lastSuccessTime": 1597910673,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        },
        "data": {
            "count": "256"
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

## MOUNTVOLUME
| REST element |                            Value                            |
|:------------:|:-----------------------------------------------------------:|
| Method       | POST                                                        |
| Type         | RAW                                                         |
| URL          | http://{{host}}/api/ibofos/v1/volumes/{{volumeName1}}/mount |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}"
    }
}
```

### REST response - successful
```
{
    "rid": "964d431e-ac03-4920-b94a-86c33c1ecacc",
    "lastSuccessTime": 1597910800,
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
        "used": 8388608
    }
}
```
### REST response - successful
```
{
    "rid": "83447d87-e5a9-4f5a-bee8-a857cb8b9aa3",
    "lastSuccessTime": 1597910808,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2040,
            "level": "WARN",
            "description": "Volume already mounted",
            "problem": "Attempt to mount a volume that is already mounted",
            "solution": "Nothing to do"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 8388608
    }
}
```

## RENAMEVOLUME
| REST element |                         Value                         |
|:------------:|:-----------------------------------------------------:|
| Method       | PATCH                                                 |
| Type         | RAW                                                   |
| URL          | http://{{host}}/api/ibofos/v1/volumes/{{volumeName1}} |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}",
        "newname": "{{volumeName1}}"
    }
}
```

### REST response - successful
```
{
    "rid": "d30a0f60-fceb-4652-889e-3dc4b374ac83",
    "lastSuccessTime": 1597910761,
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
        "used": 4194304
    }
}
```

### REST response - failed
```
{
    "rid": "1530cced-60d1-4623-b670-8854a089eb79",
    "lastSuccessTime": 1597910772,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2010,
            "level": "WARN",
            "description": "The requested volume does not exist",
            "problem": "The volume with the requested volume name or volume ID does not exist",
            "solution": "Enter the correct volume name or volume ID after checking the volume list"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 4194304
    }
}
```

## UNMOUNTVOLUME
| REST element |                            Value                            |
|:------------:|:-----------------------------------------------------------:|
| Method       | DELETE                                                      |
| Type         | RAW                                                         |
| URL          | http://{{host}}/api/ibofos/v1/volumes/{{volumeName1}}/mount |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}"
    }
}
```

### REST response - successful
```
{
    "rid": "2f6400e3-6a6f-4028-aea0-c5daa8a4f1d5",
    "lastSuccessTime": 1597910819,
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
        "used": 8388608
    }
}
```

### REST response - failed
```
{
    "rid": "157e6963-e935-4c2e-8649-fa5cd1d8b846",
    "lastSuccessTime": 1597910827,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2041,
            "level": "WARN",
            "description": "Volume already unmounted",
            "problem": "Attempt to unmount a volume that is already unmounted",
            "solution": "Nothing to do"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 8388608
    }
}
```

## UPDATEVOLUMEQOS
| REST element |                           Value                           |
|:------------:|:---------------------------------------------------------:|
| Method       | PATCH                                                     |
| Type         | RAW                                                       |
| URL          | http://{{host}}/api/ibofos/v1/volumes/{{volumeName1}}/qos |

### REST body in the request
```
{
    "param": {
        "array": "{{arrayName}}",
        "maxiops": 100,
        "maxbw": 500
    }
}
```

### REST response - successful
```
{
    "rid": "1fd40c80-6752-49be-9510-a843c08d09d5",
    "lastSuccessTime": 1597910700,
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
        "used": 4194304
    }
}
```

### REST response - failed
```
{
    "rid": "45785ae6-e619-4c8a-ac6e-18eb90952862",
    "lastSuccessTime": 1597910717,
    "result": {
        "status": {
            "module": "VolumeManager",
            "code": 2010,
            "level": "WARN",
            "description": "The requested volume does not exist",
            "problem": "The volume with the requested volume name or volume ID does not exist",
            "solution": "Enter the correct volume name or volume ID after checking the volume list"
        }
    },
    "info": {
        "capacity": 120312771380,
        "rebuildingProgress": "0",
        "situation": "NORMAL",
        "state": "NORMAL",
        "used": 4194304
    }
}
```

