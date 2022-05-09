package globals

import (
	"unicode"

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

// ToDo (mj): this function needs to exploit CLI syntax EBNF
func IsValidVolName(name string) bool {
	for _, r := range name {
		if !unicode.IsLetter(r) {
			return false
		}
	}
	return true
}
