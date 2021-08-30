package ibofos

import (
	"pnconnector/src/routers/m9k/model"
)

func SetLogLevel(xrId string, param interface{}) (model.Request, model.Response, error) {
	return loggerSender(xrId, param, "SETLOGLEVEL")
}

func GetLogLevel(xrId string, param interface{}) (model.Request, model.Response, error) {
	return loggerSender(xrId, param, "GETLOGLEVEL")
}

func ApplyLogFilter(xrId string, param interface{}) (model.Request, model.Response, error) {
	return loggerSender(xrId, param, "APPLYLOGFILTER")
}

func LoggerInfo(xrId string, param interface{}) (model.Request, model.Response, error) {
	return loggerSender(xrId, param, "LOGGERINFO")
}

func loggerSender(xrId string, param interface{}, command string) (model.Request, model.Response, error) {
	return Requester{xrId, param, model.LoggerParam{}}.Send(command)
}
