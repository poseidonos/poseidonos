package ibofos

import (
	"pnconnector/src/routers/m9k/model"
)

func PerfImpact(xrId string, param interface{}) (model.Request, model.Response, error) {
	return rebuildSender(xrId, param, "REBUILDPERFIMPACT")
}

func rebuildSender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.RebuildParam{}}.Send(command)
}
