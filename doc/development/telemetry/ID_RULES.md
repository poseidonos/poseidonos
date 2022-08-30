- [**Reserved**](#reserved)
- [**Common**](#common)
- [**CLI**](#cli)
- [**Device**](#device)
- [**Meta**](#meta)
- [**MetaFs**](#metafs)
- [**Volume**](#volume)
- [**Array**](#array)
- [**Network**](#network)
- [**Resource Monitoring**](#resource)
- [**Disk Monitoring**](#disk)

---
Rules for telemetry entry naming in "/src/telemetry/telemetry_id.h"

---
## **Reserved**
Reserved ID Range : 0 ~ 4999

---
## **Common**
Common ID Range : 5000 ~ 9999

05000 common_process_uptime_second

---
## **CLI**
CLI ID Range : 10000 ~ 19999

---

## **Device**
Device ID Range : 20000 ~ 29999

20000 read_iops_device

20001 read_bps_device

20010 write_iops_device

20011 write_bps_device

---

## **Meta**
Meta ID Range : 30000 ~ 39999

**Allocator**: 30000 ~ 32999

**Mapper**   : 33000 ~ 35999

**Journal**  : 36000 ~ 39999

---

## **MetaFs**
MetaFs ID Range : 40000 ~ 49999

40000 NormalShutdown

40100 PendingMioCount

40101 PendingMpioCount

---

## **Volume**
Volume ID Range : 50000 ~ 59999

50000 read_iops_volume

50001 read_bps_volume

50002 read_avg_lat_volume

50010 write_iops_volume

50011 write_bps_volume

50012 write_avg_lat_volume

---

## **Array**
Array ID Range : 60000 ~ 69999

60001 ArrayStatus

---

## **Network**
Array ID Range : 70000 ~ 79999

70000 read_iops_network

70001 read_bps_network

70010 write_iops_network

70011 write_bps_network

---

## **Resource**
Resource monitoring ID Range : 100000 ~ 100099

100000 Available host memory size

---

## **Disk**
Disk monitoring ID Range : 110000 ~ 110099

110000 soft media error count lower

110001 soft media error count upper

110002 power cycle count lower

110003 power cycle count upper

110004 power on hour lower

110005 power on hour upper

110006 unsafe shutdowns count lower

110007 unsafe shutdowns count upper

110008 temperature

110009 available spare

110010 available spare threshold

110011 percentage used

110012 controller busy time lower

110013 controller busy time upper

110014 warning temperature time

110015 critical temperature time

110020 lifetime waf

110021 trailing hour waf

110022 trim sector count lower

110023 trim sector count upper

110024 host written count lower

110025 host written count upper

110026 nand written count lower

110027 nand written count upper

110028 thermal throttle event count

110029 highest temperature

110030 lowest temeperature

110031 over temperature_count

110032 under temperature count

---
