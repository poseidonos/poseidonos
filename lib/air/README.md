# Getting Started with PoseidonOS

This is an AIR tutorial to profile PoseidonOS. Here is 4 Steps to use AIR:

1. Update AIR configuration
2. Insert AIR APIs
3. Build with PoseidonOS
4. Visualize AIR data

For more details, see [AIR User Guide](docs/user_guide.md) documentation.

## 1. Update configuration

First of all, AIR defines a configuration. This configuration includes system wide rules and node specific information.

Write general and node specific configuration in the file `"pos_root_dir/lib/air/config/air.cfg"`. An example:

```
R"AIR(

This configuration file is used as an AIR fundamental setting.
All of setting data would be interpreted at compile-time.
If this configuration syntax is invalid, it means build error may occur.

Paragraphs are organized within sqaure brackets at the beginning and end.
Sentences must be enclosed in double quotation marks.
In paragraph, several sentences could be exist. Here is an example.
[Paragraph1] "Sentence1" "Sentence2" "Sentence3" [/Paragraph2]
User should not change paragraphs.
User could change or add/remove sentences.


- DEFAULT paragraph contains default behavior.
  This paragraph must have only one sentence which has key:value pairs below.

  * AirBuild              {Mandatory key, Valid value: True/False}
  : False value means all AIR API do nothing and all the other options below is useless.
  So that target application might be built without AIR library at compile-time(depending on optimization).
  User can not control this option at run-time.
  
  * StreamingInterval     {Mandatory key, Valid value: 1~99 (seconds)}
  : AIR internally collects raw sampled data and calculates it to make statistics result.
  StreamingInterval key means periodic base time for calculating & saving the result.
  For example, when this value is set to 3, every 3 seconds, sampled data during 3 seconds will be calculated
  and the statistics result will be sent to the file(/tmp/air_yyyymmdd_pid.json).
  User can control this option by air_cli, air_tui at run-time.

  * NodeBuild             {Mandatory key, Valid value: True/False}
  : False value means airlog(...) API do nothing but other AIR business logic works.
  If a sentence has not this option in GROUP and NODE paragraph, that follows DEFAULT paragraph's rule.
  User can not control this option at run-time.
  
  * NodeRun               {Mandatory, Valid value: On/Off}
  : Off value means airlog(...) API stop logging. On value means airlog(...) API keep logging.
  If a sentence has not this option in GROUP and NODE paragraph, that follows DEFAULT paragraph's rule.
  User can control this option by air_cli, air_tui at run-time.

  * NodeSamplingRatio     {Mandatory, Valid value: 1~99999 (probability = 1/N)}
  : Queue type of NODE sentences may effect this sampling rule. For example, when this value is set to 1000,
  airlog(...) API actually collect raw data once a thousand.
  User can control this option by air_cli, air_tui at run-time.
  
  * NodeIndexSize         {Mandatory, Valid value: 1~999}
  : NodeIndex could be used to profiling numerical separation for the same NODE sentence.
  For example, when NodeIndexSize value is set to 10, user can distinguish 10 maximum different(hashed)
  statistics data for the same NODE sentence. The range of the NodeIndex is the same as the range of uint64_t.
  When AIR build, NodeIndexSize is one of the factor to decide memory pool size.
  User can not control this option at run-time.

[DEFAULT]
    "AirBuild : True, StreamingInterval : 3,
     NodeBuild : True, NodeRun : On, NodeSamplingRatio : 1000, NodeIndexSize : 10"
[/DEFAULT]


- GROUP paragraph defines group behavior.
  GROUP sentences may have node related behavior(NodeBuild, NodeRun, ...) such as DEFAULT sentence.
  However, it's not necessary but optional.
  If a GROUP sentence has a different value of NodeRun, group rule has more higher priority than default rule.
  So that, it follows GROUP sentence's NodeRun value. If not defined NodeRun, it follows default behavior.
  This paragraph could have several sentences which have key:value pairs below.
  
  * Group               {Mandatory, Valid value: unique string in GROUP paragraph}
  : Name of group, this value has to be unique in GROUP paragraph without space.
  User can not control this option at run-time.

  * NodeBuild           {Optional, Valid value: True/False}
  : Same as default rule

  * NodeRun             {Optional, Valid value: On/Off}
  : Same as default rule

  * NodeSamplingRatio   {Optional, Valid value: 1~99999 (probability = 1/N)}
  : Same as default rule

  * NodeIndexSize       {Optional, Valid value: 1~999}
  : Same as default rule

[GROUP]
    "Group:SUBMIT,    NodeIndexSize: 5"
    "Group:COMPLETE"
    "Group:UNGROUPED"
[/GROUP]


- FILTER paragraph defines semantic separation for the NODE sentence.
  This paragraph could have several sentences which have key:value pairs below.
  
  * Filter              {Mandatory, Valid value: unique enum name in target application}
  : Name of filter, this value is used as enum name in target application cause of C-style API extension.
  
  * Item                {Mandatory, Valid value: unique enumerator in target application}
  : This value is used as enumerator in target application. It's organized within round bracket.
  In round bracket, number of enumerators could be exist. Here is an example.
  (TYPE_A, TYPE_B, TYPE_C)
  Also, range expression with suffix(_#number) is possible. Here is an example.
  (TYPE_1 ... TYPE_30)

[FILTER]
    "Filter: AIR_Basic,     Item: (AIR_BASE)"
    "Filter: AIR_IOtype,    Item: (AIR_READ, AIR_WRITE)"
    "Filter: AIR_Thread,    Item: (AIR_SUBMIT, AIR_PROCESS, AIR_COMPLETE)"
    "Filter: AIR_Range,     Item: (AIR_0 ... AIR_3)"
[/FILTER]


- NODE paragraph defines NODE sentences that tracing point of code with specific data type.
  This paragraph could have several sentences which have key:value pairs below.
  
  * Node                {Mandatory, Valid value: unique enumerator in target application}
  : Name of node, this value is used as first parameter of airlog(...) API.
  For example, C++ style --> airlog("NodeA", ...), C style --> AIRLOG(NodeA, ...) 
  
  * Filter              {Mandatory, Valid value: Filter name}
  : Second parameter value of airlog(...) API has to be one of the Item from this Filter.
  
  * Type                {Mandatory, Valid value: Count/Latency/Performance/Queue/Utilization}
  : Forth parameter value of airlog(...) API is calculated differently according to the type value.
  Count       --> +/- value
  Latency     --> unique key for matching between start point and end point
  Performance --> io size
  Queue       --> queue depth
  Utilization --> usage(tick, size, ...)
  
  * Group               {Mandatory, Valid value: Group name}
  : If a NODE sentence doesn't have optional key/value pairs below, those rules follow this group behavior.
  If a NODE sentence has a different value of Run, node rule has more higher priority than group rule.
  
  * Build               {Optional, Valid value: True/False}
  : Same as DEFAULT rule
  
  * Run                 {Optional, Valid value: On/Off}
  : Same as DEFAULT rule
  
  * SamplingRatio       {Optional, Valid value: 1~99999 (probability = 1/N)}
  : Same as DEFAULT rule

  * IndexSize           {Optional, Valid value: 1~999}
  : Same as DEFAULT rule

[NODE]
    "Node: PERF_BENCHMARK,  Filter: AIR_IOtype,  Type: Performance,  Group: UNGROUPED"
    "Node: LAT_SUBMIT,      Filter: AIR_Range,   Type: Latency,      Group: SUBMIT"
    "Node: LAT_PROCESS,     Filter: AIR_Range,   Type: Latency,      Group: UNGROUPED"
    "Node: LAT_COMPLETE,    Filter: AIR_Range,   Type: Latency,      Group: COMPLETE"
    "Node: LAT_IO_PATH,     Filter: AIR_Range,   Type: Latency,      Group: UNGROUPED,  IndexSize: 3"
    "Node: Q_SUBMISSION,    Filter: AIR_Basic,   Type: Queue,        Group: SUBMIT"
    "Node: Q_COMPLETION,    Filter: AIR_Basic,   Type: Queue,        Group: COMPLETE,   Build: False"
    "Node: UTIL_SUBMIT_THR, Filter: AIR_Thread,  Type: Utilization,  Group: SUBMIT"
    "Node: CNT_TEST_EVENT,  Filter: AIR_Thread,  Type: Count,        Group: SUBMIT,     IndexSize: 10"
[/NODE]

)AIR"

```

##### Build AIR library

If the configuration file is updated, user has to build AIR & SPDK to adjust the changes `"./pos_root_dir/lib/build_pos_lib.sh all"`.

## 2. Insert AIR APIs

AIR has a number of APIs which user insert by oneself. If user wants to profile some point of code, user needs to insert a proper API. For more details, see [AIR User Guide](docs/user_guide.md) documentation.

### 2.1. Operation APIs

These APIs are already inserted in PoseidonOS. See the file `"pos_root_dir/src/main/poseidonos.cpp"`:

```
...
#include "Air.h"
...
int main(int argc, char *argv[])
{
	...
    air_initialize(general_core);  // AIR running core #
    air_activate();
    ...
    air_deactivate();
    air_finalize();
    
    return retVal;
}
...
```

### 2.2. Logging APIs

When AIR collects data, it can classify thread ID where the data came from. User doesn't need to concern about thread ID. AIR automatically handles it.

#### 2.2.1 Use Case 1) PoseidonOS volume performance profiling

See the file `"pos_root_dir/src/io/frontend_io/aio.cpp"`:

```
...
#include "Air.h"
...
void
AIO::SubmitAsyncIO(pos_io& posIo)
{
    ...
    case UbioDir::Write:
    {
        airlog("PERF_VOLUME", "AIR_WRITE", posIo.volume_id, posIo.length);
        ...
    }
    case UbioDir::Read:
    {
        airlog("PERF_VOLUME", "AIR_READ", posIo.volume_id, posIo.length);
        ...
    }
    ...
}
```

There are two airlog(...) APIs(C++ style) for logging performance in this file. Through this APIs, AIR can get the IOPS per volume & per thread.

#### 2.2.2 Use Case 2) SPDK bdev latency profiling

See the file `"pos_root_dir/lib/spdk-20.10/module/bdev/pos/bdev_pos.c"`:

```
...
#include "Air_c.h"
...
static int _bdev_pos_eventq_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io){
    ...
    case SPDK_BDEV_IO_TYPE_READ:
        AIRLOG(LAT_BDEV_READ, BDEV_IO_BEGIN, vol_id, (uint64_t)bdev_io);
        ...
    case SPDK_BDEV_IO_TYPE_WRITE:
        AIRLOG(LAT_BDEV_WRITE, BDEV_IO_BEGIN, vol_id, (uint64_t)bdev_io);
        ...
}
...
static void bdev_pos_io_complete(struct pos_io* io, int status)
{
    ...
    if (READ == io->ioType) {
        AIRLOG(LAT_BDEV_READ, BDEV_IO_END, io->volume_id, (uint64_t)io->context);
    }
    else if (WRITE == io->ioType) {
        AIRLOG(LAT_BDEV_WRITE, BDEV_IO_END, io->volume_id, (uint64_t)io->context);
    }
    ...
}
```

There are four AIRLOG(...) APIs(C style) for logging latency in this file. Through this APIs, AIR can get the time-lag between submission time-stamp and completion time-stamp of bdev layer.

## 3. Build with PoseidonOS

The build sequence is as follows:

1. build AIR
2. build SPDK
3. build POS

If AIR configuration file is modified, just clean PoseidonOS and try to build again. If SPDK & POS want to use AIR, AIR has to be built with configuration file(default: air_root_dir/config/air.cfg) and link the binary(static library air_root_dir/lib/libair.a).

## 4. Visualize AIR data

There is a way to visualize AIR data.

### 4.1 TUI

TUI is a primitive and direct way. User can see the result on the machine where PoseidonOS run. TUI displays the latest time result. It means you cannot check previous data when the time's gone. To run TUI, just execute `"air_tui"` on the terminal.

```
AIR TUI status: [play],  interval: [3],  timestamp: 2021-5-20:17:34:52, pid: 22232
arrow up/down: movement, arrow right/left: unfold/fold, q(esc): quit, 1~9: interval, i: init, o: run, x: stop
(*)   Top
( )   +Group:COMPLETE
( )[O]++Node:LAT_COMPLETE("latency")
      ""(0), index:0, filter:"AIR_0~AIR_1" Period(avg:107ns, median:90ns , max:745ns, sample:100.0 ), Total(avg:105ns, median:82ns , max:5us  , sample:300.0 )
( )[.]..Node:Q_COMPLETION
( )   +Group:SUBMIT
( )[O]++Node:CNT_TEST_EVENT("count")
      "CompleteIO"(22239), index:0, filter:"AIR_COMPLETE" Period(count:3.1m   ), Total(count:8.5m   )
      "CompleteIO"(22239), index:1, filter:"AIR_COMPLETE" Period(count:-2.9m  ), Total(count:-8.1m  )
( )[O]++Node:LAT_SUBMIT("latency")
      ""(0), index:0, filter:"AIR_0~AIR_1" Period(avg:306ns, median:202ns, max:1us  , sample:100.0 ), Total(avg:510ns, median:218ns, max:32us , sample:300.0 )
( )[O]++Node:Q_SUBMISSION("queue")
      "ProcessIO"(22237), index:0, filter:"AIR_BASE" Period(avg:209.7 , max:256.0 ), Total(avg:192.1 , max:256.0 )
      "ProcessIO"(22238), index:1, filter:"AIR_BASE" Period(avg:230.2 , max:256.0 ), Total(avg:213.4 , max:256.0 )
( )[O]++Node:UTIL_SUBMIT_THR("utilization")
      "SubmitIO"(22236), index:0, filter:"AIR_SUBMIT" Period(usage:33.9m , 20.47%), Total(usage:96.5m , 20.59%)
      "SubmitIO"(22236), index:1, filter:"AIR_SUBMIT" Period(usage:33.9m , 20.47%), Total(usage:96.5m , 20.59%)
      "ProcessIO"(22237), index:0, filter:"AIR_PROCESS" Period(usage:30.9m , 18.69%), Total(usage:86.2m , 18.39%)
      "ProcessIO"(22238), index:1, filter:"AIR_PROCESS" Period(usage:28.8m , 17.40%), Total(usage:82.7m , 17.67%)
      "CompleteIO"(22239), index:0, filter:"AIR_COMPLETE" Period(usage:9.2m  , 5.59%), Total(usage:25.5m , 5.44%)
      "CompleteIO"(22239), index:1, filter:"AIR_COMPLETE" Period(usage:28.8m , 17.38%), Total(usage:81.1m , 17.32%)
( )   +Group:UNGROUPED
( )[O]++Node:LAT_IO_PATH("latency")
      ""(0), index:0, filter:"AIR_0~AIR_1" Period(avg:308ns, median:158ns, max:1us  , sample:100.0 ), Total(avg:373ns, median:172ns, max:10us , sample:300.0 )
      ""(0), index:0, filter:"AIR_1~AIR_2" Period(avg:6us  , median:6us  , max:14us , sample:100.0 ), Total(avg:110us, median:110us, max:259us, sample:180.0 )
      ""(0), index:0, filter:"AIR_2~AIR_3" Period(avg:6us  , median:5us  , max:14us , sample:100.0 ), Total(avg:4us  , median:3us  , max:22us , sample:294.0 )
      ""(0), index:1, filter:"AIR_0~AIR_1" Period(avg:283ns, median:212ns, max:1us  , sample:100.0 ), Total(avg:318ns, median:176ns, max:12us , sample:289.0 )
      ""(0), index:1, filter:"AIR_1~AIR_2" Period(avg:0ns  , median:0ns  , max:0ns  , sample:0.0   ), Total(avg:336us, median:344us, max:367us, sample:75.0  )
      ""(0), index:1, filter:"AIR_2~AIR_3" Period(avg:22us , median:22us , max:27us , sample:90.0  ), Total(avg:10us , median:9us  , max:27us , sample:277.0 )
( )[O]++Node:LAT_PROCESS("latency")
      ""(0), index:0, filter:"AIR_0~AIR_1" Period(avg:648ns, median:630ns, max:6us  , sample:99.0  ), Total(avg:675ns, median:469ns, max:12us , sample:298.0 )
      ""(0), index:1, filter:"AIR_0~AIR_1" Period(avg:723ns, median:540ns, max:4us  , sample:100.0 ), Total(avg:811ns, median:617ns, max:11us , sample:300.0 )
( )[O]++Node:PERF_BENCHMARK("performance")
        SUM_Period(iops:2.0m  , bw:8.1GB  )
      "CompleteIO"(22239), index:0, filter:"AIR_READ" Period(iops:1.0m  , bw:4.2GB  , "4096(sz)-3081587(cnt)"), Total(iops_avg:943.0k, bw_avg:3.9GB  )
      "CompleteIO"(22239), index:1, filter:"AIR_READ" Period(iops:958.7k, bw:3.9GB  , "4096(sz)-2876159(cnt)"), Total(iops_avg:901.3k, bw_avg:3.7GB  )
```



In TUI, user can control AIR CLI in real-time.
