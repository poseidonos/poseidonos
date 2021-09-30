package displaymgr

import (
	"fmt"
	"pnconnector/src/log"
	"strings"
)

func AskConfirmation(message string) bool {
	var input string

	fmt.Print(message + " (y/n):")
	_, err := fmt.Scan(&input)
	if err != nil {
		log.Error("error:", err)
		panic(err)
	}

	input = strings.ToLower(input)
	input = strings.TrimSpace(input)

	if input == "yes" || input == "y" {
		return true
	}

	return false
}
