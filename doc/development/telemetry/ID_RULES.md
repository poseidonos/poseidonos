- [**Reserved**](#reserved)
- [Reserved ID Range : 0 ~ 9999](#reserved-id-range--0--9999)
- [**CLI**](#cli)
- [**Device**](#device)
- [**Meta**](#meta)
- [**MetaFs**](#metafs)
- [**Volume**](#volume)
- [**Array**](#array)
- [**Resource Monitoring**](#resource)

---
Rules for telemetry entry naming in "/src/telemetry/telemetry_id.h"

---
## **Reserved**
Reserved ID Range : 0 ~ 9999
---
## **CLI**
CLI ID Range : 10000 ~ 19999

---

## **Device**
Device ID Range : 20000 ~ 29999

20000 read_unknown_iops_per_ssd

20001 read_meta_iops_per_ssd

20002 read_gc_iops_per_ssd

20003 read_host_iops_per_ssd

20004 read_flush_iops_per_ssd

20005 read_rebuild_iops_per_ssd

20006 read_unknown_rate_bytes_per_second_per_ssd

20007 read_meta_rate_bytes_per_second_per_ssd

20008 read_gc_rate_bytes_per_second_per_ssd

20009 read_host_rate_bytes_per_second_per_ssd

20010 read_flush_rate_bytes_per_second_per_ssd

20011 read_rebuild_rate_bytes_per_second_per_ssd

20012 write_unknown_iops_per_ssd

20013 write_meta_iops_per_ssd

20014 write_gc_iops_per_ssd

20015 write_host_iops_per_ssd

20016 write_flush_iops_per_ssd

20017 write_rebuild_iops_per_ssd

20018 write_unknown_rate_bytes_per_second_per_ssd

20019 write_meta_rate_bytes_per_second_per_ssd

20020 write_gc_rate_bytes_per_second_per_ssd

20021 write_host_rate_bytes_per_second_per_ssd

20022 write_flush_rate_bytes_per_second_per_ssd

20023 write_rebuild_rate_bytes_per_second_per_ssd

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

50000 read_iops

50001 read_rate_bytes_per_second

50002 read_latency_mean_ns

50003 read_latency_max_ns

50010 write_iops

50011 write_rate_bytes_per_second

50012 write_latency_mean_ns

50013 write_latency_max_ns

---

## **Array**
Array ID Range : 60000 ~ 70000

60001 ArrayStatus

---

## **Resource monitoring**
Resource monitoring ID Range : 100000 ~ 100099

100000 Available memory size

---