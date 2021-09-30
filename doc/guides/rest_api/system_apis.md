# System APIs
## STOPPOS
| REST element | Value                                |
|:------------:|:------------------------------------:|
| Method       | DELETE                               |
| Type         |                                      |
| URL          | http://{{host}}/api/ibofos/v1/system |

### REST response - successful
```
{
    "rid": "c9487931-cfdd-4f5b-a595-d09b6ce0fe89",
    "lastSuccessTime": 1597820084,
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
    "rid": "6e4ab80b-07ee-4354-87d3-12df9821e432",
    "lastSuccessTime": 1597820066,
    "result": {
        "status": {
            "module": "system",
            "code": 9003,
            "level": "ERROR",
            "description": "The request cannot be executed since ibofos is mounted",
            "problem": "ibofos already has been mounted",
            "solution": "try again after unmount ibofos"
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

## MOUNTIBOFOS
| REST element | Value                                      |
|:------------:|--------------------------------------------|
| Method       | POST                                       |
| Type         |                                            |
| URL          | http://{{host}}/api/ibofos/v1/system/mount |

### REST response - successful
```
{
    "rid": "8f5ecc2c-7772-4081-b3b8-e0e52822dcdb",
    "lastSuccessTime": 1597819990,
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
    "rid": "85b04f05-e5d7-46c8-aa75-192f35a58a21",
    "lastSuccessTime": 1588920703,
    "result": {
        "status": {
            "module": "",
            "code": 1022,
            "description": "TIMED OUT"
        }
    },
    "info": {
        "state": "DIAGNOSIS",
        "situation": "TRY_MOUNT",
        "rebuliding_progress": 0,
        "capacity": 120795955200,
        "used": 0
    }
}
```

# RUNIBOFOS
| REST element | Value                                |
|:------------:|--------------------------------------|
| Method       | POST                                 |
| Type         |                                      |
| URL          | http://{{host}}/api/ibofos/v1/system |

### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 1597819762,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        }
    }
}
```
### REST response - failed
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "",
            "code": 11000,
            "description": "Exec command error"
        }
    },
    "info": {
        "state": "",
        "situation": "",
        "rebuliding_progress": 0,
        "capacity": 0,
        "used": 0
    }
}
```

## UNMOUNTIBOFOS
| REST element |                    Value                   |
|:------------:|:------------------------------------------:|
| Method       | DELETE                                     |
| Type         |                                            |
| URL          | http://{{host}}/api/ibofos/v1/system/mount |

### REST response - successful
```
{
    "rid": "84fd344b-774a-4733-b6a8-dcebd68dedf8",
    "lastSuccessTime": 1597820004,
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
    "rid": "742c4553-0909-43b6-97bf-6519cdee2b71",
    "lastSuccessTime": 1597819883,
    "result": {
        "status": {
            "module": "system",
            "code": 9001,
            "level": "ERROR",
            "description": "failed to unmount ibofos",
            "problem": "ibofos not mounted"
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

## IBOFOSINFO
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | GET                                  |
| Type         | RAW                                  |
| URL          | http://{{host}}/api/ibofos/v1/system |

### REST response - successful
```
{
    "rid": "d62522e2-336c-42dd-95dc-f7cd44c7e708",
    "lastSuccessTime": 1597908994,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        },
        "data": {
            "capacity": 120312771380,
            "rebuildingProgress": "0",
            "situation": "NORMAL",
            "state": "NORMAL",
            "used": 4194304
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

