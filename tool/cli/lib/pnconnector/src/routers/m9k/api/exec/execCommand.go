package exec

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
)

func ForceKillIbof(xrId string) (model.Response, error) {
	util.ExecCmd("pkill -9 ibofos", false)
	res := model.Response{}
	res.Result.Status.Code = 0
	return res, nil
}
