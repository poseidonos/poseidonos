package util

import (
	"pnconnector/src/log"
	"encoding/json"
	"github.com/google/uuid"
)

func PrettyJson(jsonByte interface{}) []byte {
	prettyJSON, err := json.MarshalIndent(jsonByte, "", "    ")

	if err != nil {
		log.Error(err)
		return nil
	}
	return prettyJSON
}

func IsValidUUID(u string) bool {
	_, err := uuid.Parse(u)
	return err == nil
}
