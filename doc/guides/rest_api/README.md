# REST API
POS offers REST APIs to be on a par with CLI commands to ease integration efforts. d-agent is POS management process that translates HTTP requests into POS CLI commands and communicates directly witih POS CLI server. The REST endpoint is hosted on Nginx with the default configuration of (port=80, timeout = 30 seconds). The schema of expected HTTP requests is as follows.

## Request Header


|Key|Value|Sample|
|---|---|---|
|X-Request-Id|{uuid4}|44f1280b-982e-4d2e-ab14-fe9eb2022045|
|ts|{unix_timestamp}|1566287153702|
|Content-Type|application/json|application/json|



## Request Body
All API has common request scheme except GET method.

### REST request schema
```
{
  "param":{
    // Ref. each command
}
```

## Response Header


| Key            | Value                   | Sample                               |
|----------------|-------------------------|--------------------------------------|
| X-Request-Id   | {the same as request's} | 44f1280b-982e-4d2e-ab14-fe9eb2022045 |
| Content-Type   | application/json        | application/json                     |
| Content-Length | {length}                | 97                                   |


## Response Body
All API has common response scheme.

### REST response schema
```
{
   "result":{
    "status":{
      "code":0,
      "description":"Some Message"
    },
    "data":{ // Optional
      // Ref. each command
    } 
  },
  "info": { // Optional
      "state": "OFFLINE",
      "situation": "DEFAULT",
      "rebuliding_progress": 0,
      "capacity": 0,
      "used": 0
  }
}
```

### SEE ALSO
- [System APIs](system_apis.md)
- [Volume APIs](volume_apis.md)
- [Metrics APIs](metrics_apis.md)
- [Internal APIs](internal_apis.md)
- [Array APIs](array_apis.md)
- [Device APIs](device_apis.md)

