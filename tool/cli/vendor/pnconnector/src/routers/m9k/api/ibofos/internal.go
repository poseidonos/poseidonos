package ibofos

import (
	"pnconnector/src/routers/m9k/model"
)

func ReportTest(xrId string, param interface{}) (model.Request, model.Response, error) {
	return internalSender(xrId, param, "REPORTTEST")
}

func StartDeviceMonitoring(xrId string, param interface{}) (model.Request, model.Response, error) {
	return internalSender(xrId, param, "STARTDEVICEMONITORING")
}

func StopDeviceMonitoring(xrId string, param interface{}) (model.Request, model.Response, error) {
	return internalSender(xrId, param, "STOPDEVICEMONITORING")
}

func DeviceMonitoringState(xrId string, param interface{}) (model.Request, model.Response, error) {
	return internalSender(xrId, param, "DEVICEMONITORINGSTATE")
}

func StopRebuilding(xrId string, param interface{}) (model.Request, model.Response, error) {
	return internalSender(xrId, param, "STOPREBUILDING")
}

func internalSender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.InternalParam{}}.Send(command)
}
