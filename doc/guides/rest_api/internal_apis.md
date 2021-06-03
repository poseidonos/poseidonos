# Internal APIs
This section contains advanced topics for developers' troubleshooting. 

## D-Agent: HEARTBEAT
You could query on d-agent to retrieve its health/version information. 

| REST element |                  Value                  |
|:------------:|:---------------------------------------:|
| Method       | GET                                     |
| Type         | RAW                                     |
| URL          | http://{{host}}/api/dagent/v1/heartbeat |

### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 1597908627,
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
            "module": "Management Stack(M9K)",
            "code": 12010,
            "level": "ERROR",
            "description": "one of iBoF service is dead"
        }
    }
}
```

## D-Agent: VERSION
You could retrieve the version of the d-agent at the following REST endpoint. 

| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         | RAW                                   |
| URL          | http://{{host}}/api/dagent/v1/version |

### REST response - succssful
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "COMMON",
            "code": 0,
            "level": "INFO",
            "description": "Success"
        },
        "data": {
            "githash": "3c25b82ae226af620a99fd8e42b921662a658219",
            "buildTime": "1597889522"
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
            "module": "COMMON",
            "code": 12020,
            "level": "ERROR",
            "description": "could not find build info"
        },
        "data": {
            "githash": "",
            "build_time": ""
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

## D-agent: KILLDAGENT
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | DELETE                               |
| Type         | RAW                                  |
| URL          | http://{{host}}/api/dagent/v1/dagent |

### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "Success"
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

### Documentation: HTML
| REST element |                    Value                   |
|:------------:|:------------------------------------------:|
| Method       | GET                                        |
| Type         |                                            |
| URL          | http://{{host}}/api/dagent/v1/doc/api.html |

### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "Success"
        },
        "data": {
            "githash": "b98039e5f8ab19351994044960cf0e27262665b4",
            "build_time": "1591338851"
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

## Documentation: Markdown
| REST element |                   Value                  |
|:------------:|:----------------------------------------:|
| Method       | GET                                      |
| Type         |                                          |
| URL          | http://{{host}}/api/dagent/v1/doc/api.md |

## POS: Event Codes
| REST element |                     Value                     |
|:------------:|:---------------------------------------------:|
| Method       | GET                                           |
| Type         |                                               |
| URL          | http://{{host}}/api/dagent/v1/doc/events.yaml |


### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "Success"
        },
        "data": {
            "StatusList": [
                {
                    "module": "",
                    "code": 0,
                    "description": "Success"
                },
                {
                    "module": "",
                    "code": 1200,
                    "description": "iBoFOS System Recovery Something"
                },
                {
                    "module": "",
                    "code": 1234,
                    "description": "iBoFOS System Recovery In Progress"
                },
                {
                    "module": "",
                    "code": 2060,
                    "description": "Fail to create volume meta"
                },
                {
                    "module": "",
                    "code": 2800,
                    "description": "Rebuild begin"
                },
                {
                    "module": "",
                    "code": 2801,
                    "description": "Rebuild done"
                },
                {
                    "module": "",
                    "code": 2082,
                    "description": "Rebuildjob added"
                },
                {
                    "module": "",
                    "code": 2083,
                    "description": "About job slicing point"
                },
                {
                    "module": "",
                    "code": 2500,
                    "description": "Array is alreday mounted"
                },
                {
                    "module": "",
                    "code": 2501,
                    "description": "Array is already umounted"
                },
                {
                    "module": "",
                    "code": 2052,
                    "description": "Array already exists"
                },
                {
                    "module": "",
                    "code": 2053,
                    "description": "Array does not exists"
                },
                {
                    "module": "",
                    "code": 2804,
                    "description": "Rebuild progress report"
                },
                {
                    "module": "",
                    "code": 2805,
                    "description": "Rebuildjob begin"
                },
                {
                    "module": "",
                    "code": 2806,
                    "description": "Rebuildjob done"
                },
                {
                    "module": "",
                    "code": 2807,
                    "description": "Rebuilding debug log"
                },
                {
                    "module": "",
                    "code": 10110,
                    "description": "Auth Error : API does not activate",
                    "solution": "Activate API"
                },
                {
                    "module": "",
                    "code": 10240,
                    "description": "Header Error : X-request-Id is invalid",
                    "solution": "Put valid value"
                },
                {
                    "module": "",
                    "code": 10250,
                    "description": "Header Error : ts missing",
                    "solution": "Put valid value"
                },
                {
                    "module": "",
                    "code": 10310,
                    "description": "Body Error : Json Error",
                    "solution": "Put valid value"
                },
                {
                    "module": "",
                    "code": 11000,
                    "description": "Exec command error"
                },
                {
                    "module": "",
                    "code": 12000,
                    "description": "iBoF is busy",
                    "solution": "Try later"
                },
                {
                    "module": "",
                    "code": 12030,
                    "description": "iBoF does not run",
                    "solution": "run iBoFOS first"
                },
                {
                    "module": "",
                    "code": 11010,
                    "description": "iBoF Unmarshal Error",
                    "solution": "Put valid value"
                },
                {
                    "module": "",
                    "code": 19000,
                    "description": "iBof Response Timeout"
                },
                {
                    "module": "",
                    "code": 11000,
                    "description": "iBof Socket Connection Error"
                },
                {
                    "module": "",
                    "code": 27756,
                    "description": "exec Run: The iBoFOS already run"
                },
                {
                    "module": "",
                    "code": 27757,
                    "description": "exec Run: Fail to run iBoFOS"
                },
                {
                    "module": "",
                    "code": 40000,
                    "description": "This API does not implemente yet"
                }
            ]
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

## POS: FORCEKILLIBOFOS
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | DELETE                               |
| Type         | RAW                                  |
| URL          | http://{{host}}/api/dagent/v1/ibofos |

### REST response - successful
```
{
    "rid": "",
    "lastSuccessTime": 0,
    "result": {
        "status": {
            "module": "",
            "code": 0,
            "description": "Success"
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

