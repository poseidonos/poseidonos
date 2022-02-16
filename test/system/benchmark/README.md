# PIBF

PIBF(PoseidonOS Integrated Benchmark Framework) is a test framework that supports to use 3rd-party benchmark tools such as fio, vdbench, and so on. PIBF abstracts and maintains many settings(kernel, poseidonos-cli, nvme-cli, ...) so that user can focus on test scenario itself.

## 1. Install requirements

If it's first time to use PIBF, you need to check your python version is higher than 3.6.

### pip setting (~/.config/pip/pip.conf) for Samsung Intranet
```
[global]
index-url = http://repo.samsungds.net:8081/artifactory/api/pypi/pypi/simple
trusted-host = repo.samsungds.net
```

### Update pip
```bash
pip3 install pip --upgrade # pip version should be higher than 21.3.1
```

### Install requirements
```bash
pip3 install -r requirements.txt
```

## 2. Setup

### Configuration
```json
{
    "Targets": [
        {
            "NAME": "Target01",
            "ID": "user",
            "PW": "password",
            "DIR": "pos_directory",
            "NIC": {
                "SSH": "nic_ip_for_ssh",
                "IP1": "nic_ip_for_test"
            },
            "PREREQUISITE": {
                "CPU": {
                    "RUN": false,
                    "SCALING": "max"
                },
                "SSD": {
                    "RUN": false,
                    "FORMAT": true
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
                    "IRQ_AFFINITYs": [
                        {
                            "NIC": "nic_interface",
                            "CPU_LIST": "core_range"
                        }
                    ],
                    "NICs": [
                        {
                            "INTERFACE": "nic_interface",
                            "IP": "nic_ip_for_test",
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
                    "HUGE_EVEN_ALLOC": "yes",
                    "NRHUGE": 65536
                },
                "DEBUG": {
                    "RUN": false,
                    "ULIMIT": "unlimited",
                    "APPORT": "disable",
                    "DUMP_DIR": "/etc/pos/core",
                    "CORE_PATTERN": "/etc/pos/core/%E.core"
                }
            },
            "SPDK": {
                "TRANSPORT": {
                    "TYPE": "tcp",
                    "NUM_SHARED_BUFFER": 4096
                },
                "SUBSYSTEMs": [
                    {
                        "NQN": "nqn.2020-10.pos\\:subsystem01",
                        "SN": "POS00000000000001",
                        "IP": "IP1",
                        "PORT": 1158
                    },
                    {
                        "NQN": "nqn.2020-10.pos\\:subsystem02",
                        "SN": "POS00000000000002",
                        "IP": "IP1",
                        "PORT": 1158
                    },
                    {
                        "NQN": "nqn.2020-10.pos\\:subsystem03",
                        "SN": "POS00000000000003",
                        "IP": "IP1",
                        "PORT": 1158
                    }
                ]
            },
            "POS": {
                "BIN": "poseidonos",
                "CLI": "poseidonos-cli",
                "CFG": "pos.conf",
                "LOG": "pos.log",
                "ARRAYs": [
                    {
                        "NAME": "ARR0",
                        "RAID_TYPE": "RAID5",
                        "USER_DEVICE_LIST": "unvme-ns-0,unvme-ns-1,unvme-ns-2",
                        "SPARE_DEVICE_LIST": "unvme-ns-3",
                        "BUFFER_DEVICE": {
                            "NAME": "uram0",
                            "TYPE": "uram",
                            "NUM_BLOCKS": 2097152,
                            "BLOCK_SIZE": 512,
                            "NUMA": 0
                        },
                        "VOLUMEs": [
                            {
                                "NAME": "VOL1",
                                "SIZE": 2147483648,
                                "SUBNQN": "nqn.2020-10.pos:subsystem01"
                            },
                            {
                                "NAME": "VOL2",
                                "SIZE": 2147483648,
                                "SUBNQN": "nqn.2020-10.pos:subsystem02"
                            },
                            {
                                "NAME": "VOL3",
                                "SIZE": 2147483648,
                                "SUBNQN": "nqn.2020-10.pos:subsystem03"
                            }
                        ]
                    }
                ]
            },
            "AUTO_GENERATE": {
                "USE": "no"
            }
        }
    ],
    "Initiators": [
        {
            "NAME": "Initiator01",
            "ID": "user",
            "PW": "password",
            "NIC": {
                "SSH": "nic_ip_for_ssh"
            },
            "SPDK": {
                "DIR": "spdk_directory",
                "TRANSPORT": "tcp"
            }
        }
    ],
    "Scenarios": [
        {
            "NAME": "fio_precommit",
            "OUTPUT_DIR": "./output"
        }
    ]
}
```

## 3. Test Benchmark

```bash
python3 benchmark.py --config [CONFIG_FILE]
```

## 4. Test unittest

```bash
python3 -m unittest # test all tc
python3 -m unittest test/specific_file.py # test specific tc
```