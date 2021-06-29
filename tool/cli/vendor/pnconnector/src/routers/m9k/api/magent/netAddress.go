package magent

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"fmt"
)

type NetAddsField struct {
	Interface string `json:"interface"`
	Address   string `json:"address"`
}

type NetAddsFields []NetAddsField

// Getting network address  and retuning JSON resonse
func GetNetAddress(param interface{}) (model.Response, error) {
	var res model.Response
	fieldsList := make(NetAddsFields, 0)
	result, err := ExecuteQuery(fmt.Sprintf(netAddQ, DBName))

	if err != nil {
		res.Result.Status, _ = util.GetStatusInfo(errQueryCode)
		res.Result.Data = make([]string, 0)
		return res, nil
	}

	if len(result) == 0 || len(result[0].Series) == 0 {
		res.Result.Status, _ = util.GetStatusInfo(errDataCode)
		res.Result.Data = make([]string, 0)
		return res, nil
	}

	for _, values := range result[0].Series[0].Values {
		if values[1] != nil {
			fieldsList = append(fieldsList, NetAddsField{values[1].(string), values[2].(string)})
		}
	}

	res.Result.Status, _ = util.GetStatusInfo(0)
	res.Result.Data = fieldsList

	return res, nil
}
