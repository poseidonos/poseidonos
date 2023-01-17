package utils

import (
	"github.com/google/uuid"
	"log"
)

func GenerateUUID() string {
	reqID, err := uuid.NewUUID()
	if err != nil {
		log.Print("Error in Generating UUID ", err.Error())
	}
	return reqID.String()
}
