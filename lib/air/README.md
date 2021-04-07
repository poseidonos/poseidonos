# Getting Started with iBoFOS

This is an AIR tutorial to profile iBoFOS. The AIR is already merged with iBoFOS devel. So what AIR user has to do in iBoFOS is:

1. Update AIR configuration
2. Insert AIR APIs
3. Build with iBoFOS
4. Visualize AIR data

For more details, see [AIR User Guide](http://10.227.30.174:7990/projects/IBOF/repos/ibofprofiling/browse/docs/user_guide.md?at=refs%2Fheads%2Fdevel) documentation.

## 1. Update configuration

 First of all, AIR defines a configuration. This configuration includes system wide rules and node specific information.

Write global and meta configuration in the file `"ibofos_root_dir/lib/air/config/air.cfg"`. An example:

```
R"AIR(    // do not change this format!!

[Default]    // default setting for Mandatory
    "StreamingInterval:1,
     AirBuild:True,
     NodeBuild:True,
     NodeRun:On,
     SamplingRatio:1000,
     AidSize:32"
[/Default]

[Group]    // group setting for Mandatory & Optional
    "GroupName: POS_IO, NodeBuild: True, NodeRun: Off"
[/Group]

[Node]    // node setting for Mandatory & Optional
    "NodeName: PERF_PSD, Type : PERFORMANCE"
    "NodeName: PERF_VOLUME, Type: PERFORMANCE, NodeBuild: True"
    "NodeName: LAT_PSD, type: LATENCY"
    "NodeName: LAT_SUBMIT, Type: LATENCY, NodeBuild: False"
    "NodeName: Q_SUBMIT, Type: QUEUE"
    "NodeName: Q_REACTOR, Type: QUEUE, NodeBuild: True"
[/Node]

[WEB]
        "NodeType: PERFORMANCE,  Type: GRAPH,  Mode: GENERAL,  Graph: LINE,  Item: [iops_read]"
        "NodeType: LATENCY,      Type: GRAPH,  Mode: GENERAL,  Graph: LINE,  Item: [mean]"
        "NodeType: QUEUE,        Type: GRAPH,  Mode: GENERAL,  Graph: LINE,  Item: [depth_period_avg]"
[/WEB]

)AIR"
```

##### Generate AIR meta file

If node_meta scope is updated, you have to build AIR & SPDK to adjust the changes `"./ibofos_root_dir/lib/build_ibof_lib.sh all"`.

## 2. Insert AIR APIs

AIR has a number of APIs which user insert by oneself. If you want to profile some point of code, you need to insert a proper API. For more details, see [AIR User Guide](http://10.227.30.174:7990/projects/IBOF/repos/ibofprofiling/browse/docs/user_guide.md?at=refs%2Fheads%2Fdevel) documentation.

### 2.1. Operation APIs

These APIs are already inserted in iBoFOS. See the file `"ibofos_root_dir/src/main/ibofos.cpp"`:

```
...
#include "air.h"
...
int main(int argc, char *argv[])
{ 
    AIR_INITIALIZE(1);  // AIR running core #
    AIR_ACTIVATE();
    ...
    AIR_DEACTIVATE();
    AIR_FINALIZE();
    
    return retVal;
}
...
```

### 2.2. Logging APIs

When AIR collects data, it can classify thread ID where the data came from. User doesn't need to concern about thread ID. AIR automatically handles it.

#### 2.2.1 Use Case 1) iBoFOS volume performance profiling

See the file `"ibofos_root_dir/src/io/frontend_io/aio.cpp"`:

```
...
#include "air.h"
...
void
AIO::SubmitAsyncIO(ibof_io* ibofIo)
{
    ...
    case UbioDir::Write:
    {
        AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_WRITE, ibofIo.length);
        ...
    }
    case UbioDir::Read:
    {
        AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_READ, ibofIo.length);
        ...
    }
    ...
}
```

There are two AIRLOG() APIs for logging performance in this file. Through this APIs, AIR can get the IOPS per volume & per thread.

#### 2.2.2 Use Case 2) SPDK bdev latency profiling

See the file `"ibofos_root_dir/lib/spdk-19.10/module/bdev/ibof/bdev_ibof.c"`:

```
...
#include "air.h"
...
static int _bdev_ibof_eventq_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io){
    ...
    case SPDK_BDEV_IO_TYPE_READ:
        AIRLOG(LAT_BDEV_READ, vol_id, 0, (uint64_t)bdev_io);
        ...
    case SPDK_BDEV_IO_TYPE_WRITE:
        AIRLOG(LAT_BDEV_WRITE, vol_id, 0, (uint64_t)bdev_io);
        ...
}
...
static void bdev_ibof_io_complete(struct ibof_io* io, int status)
{
    ...
    if (READ == io->ioType) {
        AIRLOG(LAT_BDEV_READ, io->volume_id, 1, (uint64_t)io->context);
    }
    else if (WRITE == io->ioType) {
        AIRLOG(LAT_BDEV_WRITE, io->volume_id, 1, (uint64_t)io->context);
    }
    ...
}
```

There are four AIRLOG() APIs for logging latency in this file. Through this APIs, AIR can get the latency between submission time and completion time of bdev layer.



## 3. Build with iBoFOS

The build sequence is as follows:

1. build AIR
2. build SPDK
3. build POS

Building AIR is not connected with build_ibof_lib.sh script anymore. If AIR config file is modified, just clean iBoFOS and try to build again.
If SPDK & POS want to use AIR, AIR has to be built with config file(default: .../air/config/air.cfg) and link the binary(static library .../air/lib/libair.a).


## 4. Visualize AIR data

There are two ways to visualize AIR data. User could choose what they prefer.

### 4.1 GUI

Install AIR Daemon

```bash
# cd ibofos_root_dir/lib/air/script
# ./install_daemon.sh
```

Run AIR Daemon

```bash
# service aird start
# service aird status
```

User can access to GUI through web-browser such as Firefox at their own OA PC: `"http://10.1.2.2"` or `"http://air.io"`. And user also has to set the http proxy: `12.36.76.108:3128`.

![gui](./image/readme/gui.png)

### 4.2 TUI

TUI is a primitive and direct way. User can see the result on the machine where iBoFOS run. TUI display the latest time result. It means you cannot check previous data when the time's gone. To run TUI, just execute `"air_tui"` on the terminal.

![tui](./image/readme/tui.png)

In TUI, user can control AIR CLI in real-time.
