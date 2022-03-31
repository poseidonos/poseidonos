# Configuration

PIBF(PoseidonOS Integrated Benchmark Framework) provides a **specific** JSON config file which consists of Targets, Initiators, and Scenarios.

## 1. Targets

User can set up multiple target nodes within Targets' value as a list.
```json
{
    "Targets": [
        {
            "NAME": "Target01",
            "ID": "user account",
            "PW": "user password",
            "NIC": {
                "SSH": "SSH IP addresss",
                "IP1": "Test IP1 address",
                "IP2": "Test IP2 address",
            },
            "POS": {
                "DIR": "pos root directory",
                "BIN": "pos binary name",
                "CLI": "pos CLI name",
                "CLI_LOCAL_RUN": false
            }
        }
    ]
}
```

Each object in the list is constructed as shown in the table below.

| Key          | Value  | Option    | Description                                     |
| ------------ | ------ | --------- | ----------------------------------------------- |
| NAME         | string | Mandatory | Target node name                                |
| ID           | string | Mandatory | Target node account                             |
| PW           | string | Mandatory | Target node password                            |
| NIC          | object | Mandatory | Network card IP                                 |
| PREREQUISITE | object | Optional  | Default value: `None`<br />Prerequisite setting |
| POS          | object | Mandatory | POS setting                                     |

### 1.1. NIC

TBD

### 1.2. PREREQUISITE

TBD

### 1.3. POS

TBD



## 2. Initiators

User can set up multiple initiator nodes within Initiators' value as a list.
```json
{
    "Initiators": [
        {
            "NAME": "Initiator01",
            "ID": "user account",
            "PW": "user password",
            "NIC": {
                "SSH": "SSH IP addresss"
            },
            "SPDK": {
                "DIR": "spdk root directory",
                "TRANSPORT": "tcp"
            }
        },
        {
            "NAME": "Initiator02",
            "ID": "user account",
            "PW": "user password",
            "NIC": {
                "SSH": "SSH IP addresss"
            },
            "PREREQUISITE": {
                "CPU": {
                    "RUN": false,
                    "SCALING": "max"
                },
                "MEMORY": {
                    "RUN": false,
                    "MAX_MAP_COUNT": 65535,
                    "DROP_CACHES": 3
                },
                "NETWORK": {
                    "RUN": false,
                    "IRQ_BALANCE": "stop",
                    "TCP_TUNE": "max",
                    "NICs": [
                        {
                            "INTERFACE": "Test NIC interface",
                            "IP": "Test IP address",
                            "NETMASK": 24,
                            "MTU": 9000
                        }
                    ]
                },
                "MODPROBE": {
                    "RUN": false,
                    "MODs": [
                        "nvme",
                        "nvme_core",
                        "nvme_fabrics",
                        "nvme_tcp",
                        "nvme_rdma"
                    ]
                },
                "SPDK": {
                    "RUN": false,
                    "HUGE_EVEN_ALLOC": "no",
                    "NRHUGE": 10705
                }
            },
            "SPDK": {
                "DIR": "spdk root directory",
                "TRANSPORT": "tcp"
            }
        }
    ]
}
```

Each object in the list is constructed as shown in the table below.

| Key          | Value  | Option    | Description                                     |
| ------------ | ------ | --------- | ----------------------------------------------- |
| NAME         | string | Mandatory | Initiator node name                             |
| ID           | string | Mandatory | Initiator node account                          |
| PW           | string | Mandatory | Initiator node password                         |
| NIC          | object | Mandatory | Network card IP                                 |
| PREREQUISITE | object | Optional  | Default value: `None`<br />Prerequisite setting |
| SPDK         | object | Mandatory | SPDK setting                                    |

### 2.1. NIC

| Key  | Value  | Option    | Description                                |
| ---- | ------ | --------- | ------------------------------------------ |
| SSH  | string | Mandatory | Initiator IP address to access via sshpass |

### 2.2. PREREQUISITE

| Key      | Value  | Option    | Description      |
| -------- | ------ | --------- | ---------------- |
| CPU      | object | Mandatory | CPU setting      |
| MEMORY   | object | Mandatory | Memory setting   |
| NETWORK  | object | Mandatory | Network setting  |
| MODPROBE | object | Mandatory | Modprobe setting |
| SPDK     | object | Mandatory | SPDK setting     |

#### 2.2.1. CPU

| Key     | Value   | Option    | Description                                                  |
| ------- | ------- | --------- | ------------------------------------------------------------ |
| RUN     | boolean | Mandatory | If `true`, set the below CPU option                          |
| SCALING | string  | Mandatory | Valid value: `min`, `max`<br />Set scaling_max_freq & scaling_governor |

#### 2.2.2. MEMORY

| Key           | Value   | Option    | Description                                                  |
| ------------- | ------- | --------- | ------------------------------------------------------------ |
| RUN           | boolean | Mandatory | If `true`, set the below MEMORY option                       |
| MAX_MAP_COUNT | integer | Mandatory | Set vm.max_map_count                                         |
| DROP_CACHES   | integer | Mandatory | Valid value:<br />`1` (clear page cache only)<br />`2` (clear dentries & inodes)<br />`3` (clear all)<br />Set /proc/sys/vm/drop_caches |

#### 2.2.3. NETWORK

| Key         | Value   | Option    | Description                                                  |
| ----------- | ------- | --------- | ------------------------------------------------------------ |
| RUN         | boolean | Mandatory | If `true`, set the below NETWORK option                      |
| IRQ_BALANCE | string  | Mandatory | Valid value: `start`, `stop`<br />Set irqbalacne.service     |
| TCP_TUNE    | string  | Optional  | Default value: `None`<br />Valid value: `min`, `max`<br />Set kernel tcp metrics setting |
| NICs        | list    | Mandatory | Network card setting                                         |

##### 2.2.3.1 NICs

| Key       | Value   | Option    | Description              |
| --------- | ------- | --------- | ------------------------ |
| INTERFACE | string  | Mandatory | Network card interface   |
| IP        | string  | Mandatory | Network card IP address  |
| NETMASK   | integer | Mandatory | Network card netmask bit |
| MTU       | integer | Mandatory | Network card mtu         |

#### 2.2.4. MODPROBE

| Key  | Value   | Option    | Description                              |
| ---- | ------- | --------- | ---------------------------------------- |
| RUN  | boolean | Mandatory | If `true`, set the below MODPROBE option |
| MODs | list    | Mandatory | Set modprobe                             |

#### 2.2.5. SPDK

| Key             | Value   | Option    | Description                                                  |
| --------------- | ------- | --------- | ------------------------------------------------------------ |
| RUN             | boolean | Mandatory | If `true`, set the below SPDK option                         |
| HUGE_EVEN_ALLOC | string  | Mandatory | Valid value: `yes`, `no`<br />Set spdk/scripts/setup.sh with option |
| NRHUGE          | integer | Mandatory | Set spdk/scripts/setup.sh with option<br />Number of hugepages to allocate (x 2048 byte) |

### 2.3. SPDK

| Key       | Value  | Option    | Description                |
| --------- | ------ | --------- | -------------------------- |
| DIR       | string | Mandatory | SPDK root directory        |
| TRANSPORT | string | Mandatory | Valid value: `tcp`, `rdma` |



## 3. Scenarios

User can set up multiple test scenarios within Scenarios' value as a list.
```json
{
    "Scenarios": [
        {
            "NAME": "scenario_01",
            "OUTPUT_DIR": "./output_01",
            "SUBPROC_LOG": true
        },
        {
            "NAME": "scenario_02",
            "OUTPUT_DIR": "./output_02"
        }
    ]
}
```

Each object in the list is constructed as shown in the table below.

| Key         | Value   | Option    | Description                                                  |
| ----------- | ------- | --------- | ------------------------------------------------------------ |
| NAME        | string  | Mandatory | Test scenario file name                                      |
| OUTPUT_DIR  | string  | Mandatory | Directory where all output will be stored                    |
| SUBPROC_LOG | boolean | Optional  | Default value: `false`<br />If `true`, print all subprocess message |