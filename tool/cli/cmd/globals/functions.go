package globals

import (
	"github.com/google/uuid"
	"github.com/labstack/gommon/log"
)

func GenerateUUID() string {
	uuid, err := uuid.NewUUID()
	if err != nil {
		log.Error("Failed to get a UUID:", err)
	}

	return uuid.String()
}
