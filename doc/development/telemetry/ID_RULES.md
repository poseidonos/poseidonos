- [**Reserved**](#reserved)
- [Reserved ID Range : 0 ~ 9999](#reserved-id-range--0--9999)
- [**CLI**](#cli)
- [**Device**](#device)
- [**Meta**](#meta)
- [**MetaFS**](#metafs)
- [**Volume**](#volume)
- [**Array**](#array)

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

---

## **Meta**
Meta ID Range : 30000 ~ 39999

**Allocator**: 30000 ~ 32999

**Mapper**   : 33000 ~ 35999

**Journal**  : 36000 ~ 39999

---

## **MetaFS**
MetaFS ID Range : 40000 ~ 49999

---

## **Volume**
Volume ID Range : 50000 ~ 59999

50000 ReadIops

50001 ReadBandwidth

50002 ReadLatencyMean

50003 ReadLatencyMax

50010 WriteIops

50011 WriteBandwidth

50012 WriteLatencyMean

50013 WriteLatencyMax

---

## **Array**
Array ID Range : 60000 ~ 70000

60001 ArrayStatus

60002 RebuildCounts

60003 GarbageCollectorStatus

---