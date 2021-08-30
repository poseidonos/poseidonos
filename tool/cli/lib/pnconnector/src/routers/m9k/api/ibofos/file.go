package ibofos

import (
	"pnconnector/src/routers/m9k/model"
)

func SendRequestJson(param model.Request) (model.Response, error) {
	return sendIBoF(param)
}
