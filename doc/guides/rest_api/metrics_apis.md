# Metrics APIs
POS metrics are also available for CPU/Network/Array/Volume at the following endpoints in the interval of 1m, 5m, 15m, 1h, 6h, 12h, 24h, 7d, 30d, or accumulatively since POS started up.

## METRICS: CPU
| REST element |               Value               |
|:------------:|:---------------------------------:|
| Method       | GET                               |
| Type         |                                   |
| URL          | http://{{host}}/api/metric/v1/cpu |

### REST response - successful
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
        "data": [
            {
                "time": 1597734721044941113,
                "usageUser": 9.188034187863039
            }
        ]
    }
}
```

## METRICS: CPU with Period
The unit of period could be one of 1m, 5m, 15m, 1h, 6h, 12h, 24h, 7d, or 30d.
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | GET                                  |
| Type         |                                      |
| URL          | http://{{host}}/api/metric/v1/cpu/5m |

### REST response - successful
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
        "data": [
            {
                "time": 1597734494000000000,
                "usageUser": 8.904109589275766
            },
            {
                "time": 1597734496000000000,
                "usageUser": 8.932369203964832
            },
            {
                "time": 1597734498000000000,
                "usageUser": 9.071550256103384
            },
            {
                "time": 1597734500000000000,
                "usageUser": 9.211087419587525
            },
            {
                "time": 1597734502000000000,
                "usageUser": 9.00554844300112
            }
        ]
    }
}
```

## METRICS: Latency
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         |                                       |
| URL          | http://{{host}}/api/metric/v1/latency |

### REST response - successful
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
        "data": [
            {
                "latency": 0,
                "time": 1597737402983947953
            }
        ]
    }
}
```

## METRICS: Latency with Period
| REST element |                   Value                  |
|:------------:|:----------------------------------------:|
| Method       | GET                                      |
| Type         |                                          |
| URL          | http://{{host}}/api/metric/v1/latency/5m |

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

## METRICS: Memory
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | GET                                  |
| Type         |                                      |
| URL          | http://{{host}}/api/metric/v1/memory |

### REST response - successful
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
        "data": [
            {
                "time": 1597734867980046613,
                "usageUser": 58.26696813592269
            }
        ]
    }
}
```

## METRICS: Memory with Period
| REST element |                  Value                  |
|:------------:|:---------------------------------------:|
| Method       | GET                                     |
| Type         |                                         |
| URL          | http://{{host}}/api/metric/v1/memory/5m |

### REST response - successful
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
        "data": [
            {
                "time": 1597734590000000000,
                "usageUser": 58.280068777146624
            },
            {
                "time": 1597734591000000000,
                "usageUser": 58.27984921332723
            },
            {
                "time": 1597734592000000000,
                "usageUser": 58.27962964950783
            },
            {
                "time": 1597734593000000000,
                "usageUser": 58.282093643481055
            },
            {
                "time": 1597734594000000000,
                "usageUser": 58.281190992223536
            },
            {
                "time": 1597734595000000000,
                "usageUser": 58.28163011986233
            },
            {
                "time": 1597734596000000000,
                "usageUser": 58.281386160063
            }
        ]
    }
}
```

## METRICS: Network
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         |                                       |
| URL          | http://{{host}}/api/metric/v1/network |

### REST response - successful
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
        "data": [
            {
                "time": 0,
                "bytesRecv": 83131870,
                "bytesSent": 32235833,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 303382,
                "packetsSent": 187262
            }
        ]
    }
}
```

## METRICS: Network with Period
| REST element |                   Value                  |
|:------------:|:----------------------------------------:|
| Method       | GET                                      |
| Type         |                                          |
| URL          | http://{{host}}/api/metric/v1/network/5m |

## REST response - successful
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
        "data": [
            {
                "time": 1597738402000000000,
                "bytesRecv": 41148503,
                "bytesSent": 16558355.75,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 147997.75,
                "packetsSent": 97631.75
            },
            {
                "time": 1597738403000000000,
                "bytesRecv": 41148503,
                "bytesSent": 16558355.75,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 147997.75,
                "packetsSent": 97631.75
            },
            {
                "time": 1597738404000000000,
                "bytesRecv": 41148708,
                "bytesSent": 16558355.75,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 147998.75,
                "packetsSent": 97631.75
            },
            {
                "time": 1597738405000000000,
                "bytesRecv": 41149103.5,
                "bytesSent": 16558355.75,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 148000.25,
                "packetsSent": 97631.75
            },
            {
                "time": 1597738406000000000,
                "bytesRecv": 41149308.5,
                "bytesSent": 16558355.75,
                "dropIn": 0,
                "dropOut": 0,
                "packetsRecv": 148001.25,
                "packetsSent": 97631.75
            }
        ]
    }
}
```

## METRICS: Read B/W
| REST element |                 Value                |
|:------------:|:------------------------------------:|
| Method       | GET                                  |
| Type         |                                      |
| URL          | http://{{host}}/api/metric/v1/readbw |

### REST response - successful
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
        "data": [
            {
                "bw": 22313472,
                "time": 1597735688784524383
            }
        ]
    }
}
```

## METRICS: Read B/W with Period
| REST element |                  Value                  |
|:------------:|:---------------------------------------:|
| Method       | GET                                     |
| Type         |                                         |
| URL          | http://{{host}}/api/metric/v1/readbw/5m |

### REST response - successful
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
        "data": [
            {
                "bw": 0,
                "time": 1597739252000000000
            },
            {
                "bw": 0,
                "time": 1597739253000000000
            },
            {
                "bw": 0,
                "time": 1597739254000000000
            },
            {
                "bw": 35348480,
                "time": 1597739255000000000
            },
            {
                "bw": 45733376,
                "time": 1597739256000000000
            },
            {
                "bw": 23500288,
                "time": 1597739257000000000
            },
            {
                "bw": 11842048,
                "time": 1597739258000000000
            },
            {
                "bw": 14945280,
                "time": 1597739259000000000
            },
            {
                "bw": 15092224,
                "time": 1597739260000000000
            },
            {
                "bw": 23834624,
                "time": 1597739261000000000
            }
        ]
    }
}
```

## METRICS: Read IOPS
| REST element |                  Value                 |
|:------------:|:--------------------------------------:|
| Method       | GET                                    |
| Type         |                                        |
| URL          | http://{{host}}/api/metric/v1/readiops |

### REST response - successful
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
        "data": [
            {
                "iops": 1633,
                "time": 1597736262767388762
            }
        ]
    }
}
```

## METRICS: Read IOPS with Period
| REST element |                   Value                   |
|:------------:|:-----------------------------------------:|
| Method       | GET                                       |
| Type         |                                           |
| URL          | http://{{host}}/api/metric/v1/readiops/5m |

### REST response - successful
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
        "data": [
            {
                "iops": 0,
                "time": 1597736236000000000
            },
            {
                "iops": 6376,
                "time": 1597736237000000000
            },
            {
                "iops": 23973.25,
                "time": 1597736238000000000
            },
            {
                "iops": 96155,
                "time": 1597736239000000000
            },
            {
                "iops": 0,
                "time": 1597736240000000000
            },
            {
                "iops": 0,
                "time": 1597736241000000000
            },
            {
                "iops": 0,
                "time": 1597736242000000000
            },
            {
                "iops": 0,
                "time": 1597736243000000000
            },
            {
                "iops": 19665,
                "time": 1597736244000000000
            }
        ]
    }
}
```

## METRICS: REBUILD LOG
| REST element |                     Value                    |
|:------------:|:--------------------------------------------:|
| Method       | GET                                          |
| Type         |                                              |
| URL          | http://{{host}}/api/metric/v1/rebuildlogs/5m |

### REST response - successful
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
        "data": [
            {
                "Time": 1597743919603338213,
                "Value": "[1596194025][1234][trace] progress report: [100]"
            }
        ]
    }
}
```

## METRICS: Volume Latency
| REST element |                      Value                      |
|:------------:|:-----------------------------------------------:|
| Method       | GET                                             |
| Type         |                                                 |
| URL          | http://{{host}}/api/metric/v1/volumes/1/latency |

### REST response - successful
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
        "data": [
            {
                "latency": 0,
                "time": 1597737402983947953
            }
        ]
    }
}
```

## METRICS: Volume Latency with Period
| REST element |                        Value                       |
|:------------:|:--------------------------------------------------:|
| Method       | GET                                                |
| Type         |                                                    |
| URL          | http://{{host}}/api/metric/v1/volumes/1/latency/5m |

### REST response - successful
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
        "data": [
            {
                "latency": 0,
                "time": 1597736236000000000
            },
            {
                "latency": 6376,
                "time": 1597736237000000000
            },
            {
                "latency": 23973.25,
                "time": 1597736238000000000
            },
            {
                "latency": 96155,
                "time": 1597736239000000000
            },
            {
                "latency": 0,
                "time": 1597736240000000000
            },
            {
                "latency": 0,
                "time": 1597736241000000000
            },
            {
                "latency": 0,
                "time": 1597736242000000000
            },
            {
                "latency": 0,
                "time": 1597736243000000000
            },
            {
                "latency": 19665,
                "time": 1597736244000000000
            }
        ]
    }
}
```

## METRICS: Volume Read B/W
| REST element |                      Value                     |
|:------------:|:----------------------------------------------:|
| Method       | GET                                            |
| Type         |                                                |
| URL          | http://{{host}}/api/metric/v1/volumes/1/readbw |

### REST response - successful
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
        "data": [
            {
                "bw": 0,
                "time": 1597735928875564783
            }
        ]
    }
}
```

## METRICS: Volume Read B/W with Period
| REST element |                       Value                       |
|:------------:|:-------------------------------------------------:|
| Method       | GET                                               |
| Type         |                                                   |
| URL          | http://{{host}}/api/metric/v1/volumes/1/readbw/5m |

### REST response - successful
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
        "data": [
            {
                "bw": 0,
                "time": 1597735678000000000
            },
            {
                "bw": 0,
                "time": 1597735679000000000
            },
            {
                "bw": 0,
                "time": 1597735680000000000
            },
            {
                "bw": 28773888,
                "time": 1597735681000000000
            },
            {
                "bw": 53703680,
                "time": 1597735682000000000
            }
        ]
    }
}
```

## METRICS: Volume Read IOPS
| REST element |                       Value                      |
|:------------:|:------------------------------------------------:|
| Method       | GET                                              |
| Type         |                                                  |
| URL          | http://{{host}}/api/metric/v1/volumes/1/readiops |

### REST response - successful
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
        "data": [
            {
                "iops": 0,
                "time": 1597737402983947953
            }
        ]
    }
}
```

## METRICS: Volume Read IOPS with Period
| REST element |                        Value                        |
|:------------:|:---------------------------------------------------:|
| Method       | GET                                                 |
| Type         |                                                     |
| URL          | http://{{host}}/api/metric/v1/volumes/1/readiops/5m |

### REST response - successful

## METRICS: Volume Write B/W
| REST element | Value |
|:------------:|:-----:|
| Method       | GET   |
| Type         |       |
| URL          |       |

### REST response - successful
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
        "data": [
            {
                "iops": 406,
                "time": 1597737213000000000
            },
            {
                "iops": 394,
                "time": 1597737214000000000
            },
            {
                "iops": 800,
                "time": 1597737215000000000
            },
            {
                "iops": 800,
                "time": 1597737216000000000
            },
            {
                "iops": 1645,
                "time": 1597737217000000000
            },
            {
                "iops": 1635,
                "time": 1597737218000000000
            },
            {
                "iops": 1504,
                "time": 1597737219000000000
            },
            {
                "iops": 129,
                "time": 1597737220000000000
            }
        ]
    }
}
```

## METRICS: Volume Write B/W
| REST element |                      Value                      |
|:------------:|:-----------------------------------------------:|
| Method       | GET                                             |
| Type         |                                                 |
| URL          | http://{{host}}/api/metric/v1/volumes/1/writebw |

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

## METRICS: Volume Write B/W IOPS
| REST element |                       Value                       |
|:------------:|:-------------------------------------------------:|
| Method       | GET                                               |
| Type         |                                                   |
| URL          | http://{{host}}/api/metric/v1/volumes/1/writeiops |

### REST response - successful 
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
        "data": [
            {
                "iops": 0,
                "time": 1597737402983947953
            }
        ]
    }
}
```

## METRICS: Volume Write B/W IOPS with Period
| REST element |                         Value                        |
|:------------:|:----------------------------------------------------:|
| Method       | GET                                                  |
| Type         |                                                      |
| URL          | http://{{host}}/api/metric/v1/volumes/1/writeiops/5m |

### REST response - successful
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
        "data": [
            {
                "iops": 28927,
                "time": 1597737831000000000
            },
            {
                "iops": 9124,
                "time": 1597737832000000000
            },
            {
                "iops": 0,
                "time": 1597737833000000000
            },
            {
                "iops": 25600,
                "time": 1597737834000000000
            },
            {
                "iops": 25600,
                "time": 1597737835000000000
            },
            {
                "iops": 0,
                "time": 1597737836000000000
            },
            {
                "iops": 12800,
                "time": 1597737837000000000
            },
            {
                "iops": 800,
                "time": 1597737838000000000
            },
            {
                "iops": 800,
                "time": 1597737839000000000
            }
        ]
    }
}
```

## METRICS: Volume Write B/W with Period
| REST element |                        Value                       |
|:------------:|:--------------------------------------------------:|
| Method       | GET                                                |
| Type         |                                                    |
| URL          | http://{{host}}/api/metric/v1/volumes/1/writebw/5m |

### REST response - successful
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
        "data": [
            {
                "bw": 0,
                "time": 1597736239000000000
            },
            {
                "bw": 7731712,
                "time": 1597736240000000000
            },
            {
                "bw": 26097664,
                "time": 1597736241000000000
            },
            {
                "bw": 24965632,
                "time": 1597736242000000000
            },
            {
                "bw": 26224128,
                "time": 1597736243000000000
            },
            {
                "bw": 19838464,
                "time": 1597736244000000000
            },
            {
                "bw": 0,
                "time": 1597736245000000000
            },
            {
                "bw": 0,
                "time": 1597736246000000000
            }
        ]
    }
}
```

## METRICS: Write B/W
| REST element |                 Value                 |
|:------------:|:-------------------------------------:|
| Method       | GET                                   |
| Type         |                                       |
| URL          | http://{{host}}/api/metric/v1/writebw |

### REST response - successful
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
        "data": [
            {
                "bw": 53438976,
                "time": 1597735948079139123
            }
        ]
    }
}
```

## METRICS: Write B/W with Period
| REST element |                   Value                  |
|:------------:|:----------------------------------------:|
| Method       | GET                                      |
| Type         |                                          |
| URL          | http://{{host}}/api/metric/v1/writebw/5m |

### REST response - successful
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
        "data": [
            {
                "bw": 0,
                "time": 1597735793000000000
            },
            {
                "bw": 29779968,
                "time": 1597735794000000000
            },
            {
                "bw": 26093568,
                "time": 1597735795000000000
            },
            {
                "bw": 28407808,
                "time": 1597735796000000000
            },
            {
                "bw": 0,
                "time": 1597735797000000000
            },
            {
                "bw": 0,
                "time": 1597735798000000000
            },
            {
                "bw": 6858752,
                "time": 1597735799000000000
            },
            {
                "bw": 0,
                "time": 1597735800000000000
            },
            {
                "bw": 0,
                "time": 1597735801000000000
            },
            {
                "bw": 17127936,
                "time": 1597735802000000000
            },
            {
                "bw": 21874688,
                "time": 1597735803000000000
            },
            {
                "bw": 21850112,
                "time": 1597735804000000000
            }
        ]
    }
}
```

## METRICS: Write IOPS
| REST element |                  Value                  |
|:------------:|:---------------------------------------:|
| Method       | GET                                     |
| Type         |                                         |
| URL          | http://{{host}}/api/metric/v1/writeiops |

### REST response - successful
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
        "data": [
            {
                "iops": 0,
                "time": 1597737402983947953
            }
        ]
    }
}
```

## METRICS: Write IOPS with Period
| REST element |                    Value                   |
|:------------:|:------------------------------------------:|
| Method       | GET                                        |
| Type         |                                            |
| URL          | http://{{host}}/api/metric/v1/writeiops/5m |

### REST response - successful
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
        "data": [
            {
                "iops": 0,
                "time": 1597737381000000000
            },
            {
                "iops": 47193,
                "time": 1597737382000000000
            },
            {
                "iops": 33480,
                "time": 1597737383000000000
            },
            {
                "iops": 0,
                "time": 1597737384000000000
            },
            {
                "iops": 0,
                "time": 1597737385000000000
            },
            {
                "iops": 20450,
                "time": 1597737386000000000
            },
            {
                "iops": 31656,
                "time": 1597737387000000000
            },
            {
                "iops": 27805,
                "time": 1597737388000000000
            },
            {
                "iops": 22408,
                "time": 1597737389000000000
            }
        ]
    }
}
```
