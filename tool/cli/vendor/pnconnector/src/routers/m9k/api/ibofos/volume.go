package ibofos

import (
	"pnconnector/src/errors"
	"pnconnector/src/influxdb"
	"pnconnector/src/routers/m9k/model"
)

func CreateVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	req, res, err := Requester{xrId, param, model.VolumeParam{}}.Send("CREATEVOLUME")

	if err != nil {
		return req, res, err
	}

	// Calling a go routine to mark the creation time of a volume in influxdb
	// The creation time is used in querying the influxdb for volume level data
	// This is currently a workaround for the issue that POS is crating duplicate volume Ids
	// The function is made as a go routine so that it does not cause much sde effects to the Create Volume API
	go influxdb.CreateVolume(ListVolume, param, res)

	return req, res, err
}

func MountVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.VolumeParam{}}.Send("MOUNTVOLUME")
}

func UnmountVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.VolumeParam{}}.Send("UNMOUNTVOLUME")
}

func DeleteVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	req, res, err := Requester{xrId, param, model.VolumeParam{}}.Send("DELETEVOLUME")

	if err != nil {
		return req, res, err
	}

	err = influxdb.DeleteVolume()

	if err != nil {
		err = errors.New("Request Success, but Influx Error : " + err.Error())
	}
	return req, res, err
}

func ListVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	return volumeSender(xrId, param, "LISTVOLUME")
}

func RenameVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	return volumeSender(xrId, param, "RENAMEVOLUME")
}

func ResizeVolume(xrId string, param interface{}) (model.Request, model.Response, error) {
	return volumeSender(xrId, param, "RESIZEVOLUME")
}

func GetMaxVolumeCount(xrId string, param interface{}) (model.Request, model.Response, error) {
	return volumeSender(xrId, param, "GETMAXVOLUMECOUNT")
}

func GetHostNQN(xrId string, param interface{}) (model.Request, model.Response, error) {
	return volumeSender(xrId, param, "GETHOSTNQN")
}

func volumeSender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.VolumeParam{}}.Send(command)
}
