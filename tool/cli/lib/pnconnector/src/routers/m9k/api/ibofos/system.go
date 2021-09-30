package ibofos

import (
	"fmt"
	"os"
	"path/filepath"
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/setting"
	"pnconnector/src/util"
	"time"
)

func StopPosCommand(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "STOPPOS")
}

func RuniBoFOS(xrId string, param interface{}) (model.Request, model.Response, error) {
	iBoFRequest := model.Request{
		Command: "RUNIBOFOS",
		Rid:     xrId,
	}

	iBoFRequest.Param = param
	res := model.Response{}

	path, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	cmd := fmt.Sprintf("/../script/run_remote_ibofos.sh %s", setting.Config.Server.IBoF.IP)
	err := util.ExecCmd(path+cmd, false)

	if err != nil {
		res.Result.Status.Code = 11000
	} else {
		res.Result.Status.Code = 0
		res.LastSuccessTime = time.Now().UTC().Unix()
	}

	log.Info("RuniBoFOS result : ", res.Result.Status.Code)

	return iBoFRequest, res, err
}

func IBoFOSInfo(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "GETIBOFOSINFO")
}

func IBoFOSVersion(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "GETVERSION")
}

func MountiBoFOS(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "MOUNTIBOFOS")
}

func UnmountiBoFOS(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "UNMOUNTIBOFOS")
}

func WBT(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "WBT")
}

func ListWBT(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "LISTWBT")
}

func DoGC(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "DOGC")
}

func DetachDevice(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "DETACHDEVICE")
}

func systemSender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.SystemParam{}}.Send(command)
}
