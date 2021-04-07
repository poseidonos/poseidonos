package m9k

import (
	"pnconnector/src/routers/m9k/api/exec"
	amoduleIBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	amoduleMagent "pnconnector/src/routers/m9k/api/magent"
	"pnconnector/src/routers/m9k/model"
	"dagent/src/routers/m9k/api/dagent"
	"dagent/src/routers/m9k/api/ibofos"
	"dagent/src/routers/m9k/api/magent"
	"dagent/src/routers/m9k/middleware"
	"github.com/gin-gonic/gin"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

func Route(router *gin.Engine) {
	uri := router.Group("/api")

	// Doc Static
	dir, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	dir = strings.ReplaceAll(dir, "/bin", "/doc")
	uri.StaticFS("/dagent/v1/doc", http.Dir(dir))

	//uri.Use(middleware.CheckBasicAuth())
	//uri.Use(middleware.CheckAPIActivate())
	uri.Use(middleware.CheckHeader)
	uri.Use(middleware.CheckBody)
	uri.Use(middleware.ResponseHeader)

	// D-Agent
	dagentPath := uri.Group("/dagent/v1")
	{
		dagentPath.GET("/heartbeat", func(ctx *gin.Context) {
			dagent.CallDagent(ctx, dagent.HeartBeat)
		})
		dagentPath.GET("/version", func(ctx *gin.Context) {
			dagent.CallDagent(ctx, dagent.Version)
		})
		dagentPath.DELETE("/dagent", func(ctx *gin.Context) {
			dagent.CallDagent(ctx, dagent.KillDAgent)
		})
		dagentPath.DELETE("/ibofos", func(ctx *gin.Context) {
			dagent.CallDagent(ctx, exec.ForceKillIbof)
		})
	}

	// iBoFOSPath
	iBoFOSPath := uri.Group("/ibofos/v1")
	iBoFOSPath.Use(middleware.PostHandler)

	// System
	{
		iBoFOSPath.POST("/system", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.RuniBoFOS)
		})
		iBoFOSPath.DELETE("/system", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.ExitiBoFOS)
		})
		iBoFOSPath.POST("/system/mount", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.MountiBoFOS)
		})
		iBoFOSPath.GET("/system", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.IBoFOSInfo)
		})
		iBoFOSPath.DELETE("/system/mount", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.UnmountiBoFOS)
		})
	}

	// Device
	{
		//// Temp1
		//iBoFOSPath.POST("/devices", func(ctx *gin.Context) {
		//	// Temp workaround
		//	req := model.Request{}
		//	ctx.ShouldBindBodyWith(&req, binding.JSON)
		//	marshalled, _ := json.Marshal(req.Param)
		//	param := model.DeviceParam{}
		//	_ = json.Unmarshal(marshalled, &param)
		//	param.Spare = param.Spare
		//	ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.AddDevice, param)
		//
		//	//ibofos.CalliBoFOS(ctx, amoduleIBoFOS.AddDevice)
		//})
		//// Temp2
		//iBoFOSPath.DELETE("/devices/:deviceName", func(ctx *gin.Context) {
		//	deviceName := ctx.Param("deviceName")
		//	param := model.DeviceParam{Spare: deviceName}
		//	ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.RemoveDevice, param)
		//})

		iBoFOSPath.GET("/devices", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.ListDevice)
		})
		iBoFOSPath.GET("/devices/:deviceName/scan", func(ctx *gin.Context) {
			deviceName := ctx.Param("deviceName")
			if deviceName == "all" {
				ibofos.CalliBoFOS(ctx, amoduleIBoFOS.ScanDevice)
			} else {
				// 404 return
			}
		})
		iBoFOSPath.GET("/devices/:deviceName/smart", func(ctx *gin.Context) {
			deviceName := ctx.Param("deviceName")
			param := model.DeviceParam{Name: deviceName}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.GetSMART, param)
		})
	}

	// Array
	{
		iBoFOSPath.POST("/array", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Name = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.CreateArray, param)
		})
		iBoFOSPath.GET("/array/:arrayName", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Name = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.ArrayInfo, param)
		})
		iBoFOSPath.DELETE("/array/:arrayName", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Name = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.DeleteArray, param)
		})
		iBoFOSPath.GET("/array/:arrayName/devices", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Name = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.ListArrayDevice, param)
		})
		iBoFOSPath.GET("/array/:arrayName/load", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Name = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.LoadArray, param)
		})
		iBoFOSPath.POST("/array/:arrayName/devices", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Array = ctx.Param("arrayName")
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.AddDevice, param)
		})
		iBoFOSPath.DELETE("/array/:arrayName/devices/:deviceName", func(ctx *gin.Context) {
			param := model.ArrayParam{}
			param.Array = ctx.Param("arrayName")
			param.Spare = []model.Device{{DeviceName: ctx.Param("deviceName")}}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.RemoveDevice, param)
		})
	}

	// Volume
	{
		iBoFOSPath.POST("/volumes", func(ctx *gin.Context) {
			if multiVolRes, ok := dagent.IsMultiVolume(ctx); ok {
				dagent.ImplementAsyncMultiVolume(ctx, amoduleIBoFOS.CreateVolume, &multiVolRes, dagent.CREATE_VOLUME)
			} else {
				ibofos.CalliBoFOS(ctx, amoduleIBoFOS.CreateVolume)
			}
		})
		iBoFOSPath.GET("/volumes", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.ListVolume)
		})
		iBoFOSPath.PATCH("/volumes/:volumeName", func(ctx *gin.Context) {
			volumeName := ctx.Param("volumeName")
			param := model.VolumeParam{Name: volumeName}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.RenameVolume, param)
		})
		iBoFOSPath.GET("/volumes/maxcount", func(ctx *gin.Context) {
			ibofos.CalliBoFOS(ctx, amoduleIBoFOS.GetMaxVolumeCount)
		})
		iBoFOSPath.DELETE("/volumes/:volumeName", func(ctx *gin.Context) {
			volumeName := ctx.Param("volumeName")
			param := model.VolumeParam{Name: volumeName}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.DeleteVolume, param)
		})
		iBoFOSPath.POST("/volumes/:volumeName/mount", func(ctx *gin.Context) {
			if multiVolRes, ok := dagent.IsMultiVolume(ctx); ok {
				dagent.ImplementAsyncMultiVolume(ctx, amoduleIBoFOS.MountVolume, &multiVolRes, dagent.MOUNT_VOLUME)
			} else {
				volumeName := ctx.Param("volumeName")
				param := model.VolumeParam{Name: volumeName}
				ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.MountVolume, param)
			}
		})
		iBoFOSPath.DELETE("/volumes/:volumeName/mount", func(ctx *gin.Context) {
			volumeName := ctx.Param("volumeName")
			param := model.VolumeParam{Name: volumeName}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.UnmountVolume, param)
		})
		iBoFOSPath.PATCH("/volumes/:volumeName/qos", func(ctx *gin.Context) {
			volumeName := ctx.Param("volumeName")
			param := model.VolumeParam{Name: volumeName}
			ibofos.CalliBoFOSwithParam(ctx, amoduleIBoFOS.UpdateVolumeQoS, param)
		})
	}

	// MAgentPath
	mAgentPath := uri.Group("/metric/v1")
	{
		mAgentPath.GET("/cpu/", func(ctx *gin.Context) {
			param := model.MAgentParam{}
			magent.CallMagent(ctx, amoduleMagent.GetCPUData, param)
		})
		mAgentPath.GET("/cpu/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time}
			magent.CallMagent(ctx, amoduleMagent.GetCPUData, param)
		})
		mAgentPath.GET("/devices", func(ctx *gin.Context) {
			param := model.MAgentParam{}
			magent.CallMagent(ctx, amoduleMagent.GetDeviceData, param)
		})

		mAgentPath.GET("/devices/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time}
			magent.CallMagent(ctx, amoduleMagent.GetDeviceData, param)
		})

		mAgentPath.GET("/memory/", func(ctx *gin.Context) {
			param := model.MAgentParam{}
			magent.CallMagent(ctx, amoduleMagent.GetMemoryData, param)
		})

		mAgentPath.GET("/memory/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time}
			magent.CallMagent(ctx, amoduleMagent.GetMemoryData, param)
		})

		mAgentPath.GET("/network/", func(ctx *gin.Context) {
			param := model.MAgentParam{}
			magent.CallMagent(ctx, amoduleMagent.GetNetData, param)
		})

		mAgentPath.GET("/network/:networkfield", func(ctx *gin.Context) {
			networkfield := ctx.Param("networkfield")
			if networkfield == "driver" {
				param := model.MAgentParam{}
				magent.CallMagent(ctx, amoduleMagent.GetNetDriver, param)
			} else if networkfield == "hardwareaddress" {
				param := model.MAgentParam{}
				magent.CallMagent(ctx, amoduleMagent.GetNetAddress, param)
			} else {
				param := model.MAgentParam{Time: networkfield}
				magent.CallMagent(ctx, amoduleMagent.GetNetData, param)
			}
		})

		mAgentPath.GET("/readbw/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time, Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetReadBandwidth, param)
		})

		mAgentPath.GET("/readbw/", func(ctx *gin.Context) {
			param := model.MAgentParam{Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetReadBandwidth, param)
		})

		mAgentPath.GET("/volumes/:volid/readbw/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			volid := ctx.Param("volid")
			param := model.MAgentParam{Time: time, Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetReadBandwidth, param)
		})

		mAgentPath.GET("/volumes/:volid/readbw/", func(ctx *gin.Context) {
			volid := ctx.Param("volid")
			param := model.MAgentParam{Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetReadBandwidth, param)
		})

		mAgentPath.GET("/writebw/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time, Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetWriteBandwidth, param)
		})

		mAgentPath.GET("/writebw/", func(ctx *gin.Context) {
			param := model.MAgentParam{Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetWriteBandwidth, param)
		})

		mAgentPath.GET("/volumes/:volid/writebw/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			volid := ctx.Param("volid")
			param := model.MAgentParam{Time: time, Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetWriteBandwidth, param)
		})

		mAgentPath.GET("/volumes/:volid/writebw/", func(ctx *gin.Context) {
			volid := ctx.Param("volid")
			param := model.MAgentParam{Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetWriteBandwidth, param)
		})

		mAgentPath.GET("/readiops/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time, Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetReadIOPS, param)
		})

		mAgentPath.GET("/readiops/", func(ctx *gin.Context) {
			param := model.MAgentParam{Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetReadIOPS, param)
		})

		mAgentPath.GET("/volumes/:volid/readiops/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			volid := ctx.Param("volid")
			param := model.MAgentParam{Time: time, Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetReadIOPS, param)
		})

		mAgentPath.GET("/volumes/:volid/readiops/", func(ctx *gin.Context) {
			volid := ctx.Param("volid")
			param := model.MAgentParam{Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetReadIOPS, param)
		})

		mAgentPath.GET("/writeiops/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time, Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetWriteIOPS, param)
		})

		mAgentPath.GET("/writeiops/", func(ctx *gin.Context) {
			param := model.MAgentParam{Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetWriteIOPS, param)
		})

		mAgentPath.GET("/volumes/:volid/writeiops/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			volid := ctx.Param("volid")
			param := model.MAgentParam{Time: time, Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetWriteIOPS, param)
		})

		mAgentPath.GET("/volumes/:volid/writeiops/", func(ctx *gin.Context) {
			volid := ctx.Param("volid")
			param := model.MAgentParam{Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetWriteIOPS, param)
		})

		mAgentPath.GET("/latency/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time, Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetLatency, param)
		})

		mAgentPath.GET("/latency/", func(ctx *gin.Context) {
			param := model.MAgentParam{Level: "array"}
			magent.CallMagent(ctx, amoduleMagent.GetLatency, param)
		})

		mAgentPath.GET("/volumes/:volid/latency/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			volid := ctx.Param("volid")
			param := model.MAgentParam{Time: time, Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetLatency, param)
		})

		mAgentPath.GET("/volumes/:volid/latency/", func(ctx *gin.Context) {
			volid := ctx.Param("volid")
			param := model.MAgentParam{Level: volid}
			magent.CallMagent(ctx, amoduleMagent.GetLatency, param)
		})

		mAgentPath.GET("/rebuildlogs/:time", func(ctx *gin.Context) {
			time := ctx.Param("time")
			param := model.MAgentParam{Time: time}
			magent.CallMagent(ctx, amoduleMagent.GetRebuildLogs, param)
		})
	}
}
