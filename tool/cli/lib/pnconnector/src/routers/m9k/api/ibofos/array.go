package ibofos

import (
	"pnconnector/src/routers/m9k/model"
)

func ListArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "LISTARRAY")
}

func ListArrayDevice(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "LISTARRAYDEVICE")
}

func LoadArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "LOADARRAY")
}

func CreateArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "CREATEARRAY")
}

func DeleteArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "DELETEARRAY")
}

func ArrayInfo(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "ARRAYINFO")
}

func AddDevice(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "ADDDEVICE")
}

func RemoveDevice(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "REMOVEDEVICE")
}

func arraySender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.ArrayParam{}}.Send(command)
}

func MountArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "MOUNTARRAY")
}

func UnmountArray(xrId string, param interface{}) (model.Request, model.Response, error) {
	return systemSender(xrId, param, "UNMOUNTARRAY")
}

func ResetMbr(xrId string, param interface{}) (model.Request, model.Response, error) {
	return arraySender(xrId, param, "RESETMBR")
}