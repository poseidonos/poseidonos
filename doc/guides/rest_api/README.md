# REST API
POS provides its commands via REST APIs for the integration of 3rd party services with POS. To access POS via REST APIs, you need to install [Management Stack (M9K)](https://github.com/poseidonos/poseidonos-gui) first. M9K provides three major functionalities: poseidonos-gui, M-Agent, and D-agent. poseidonos-gui provides a graphical user interface to control and monitor POS. M-Agent is the interface between poseidonos-gui and AIR, which enables the performance monitoring in posiedonos-gui.

D-Agent is the interface between REST APIs and POS CLI server. The REST endpoint is hosted on Nginx with the default configuration of (port=80, timeout = 30 seconds). The RESTful request schema is as below. For the full information about the POS REST APIs, please refer to [PoseidonOS RESTful API List](https://github.com/poseidonos/poseidonos-gui/blob/main/src/dagent/doc/api.md).

## Request Header
|      Key     |       Value      |                Sample                |
|:------------:|:----------------:|:------------------------------------:|
| X-Request-Id |      {uuid4}     | 44f1280b-982e-4d2e-ab14-fe9eb2022045 |
|      ts      | {unix_timestamp} |             1566287153702            |
| Content-Type | application/json |           application/json           |


## Request Body
Every API has a common request scheme except for GET method.

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
Every API has a common response scheme.

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
