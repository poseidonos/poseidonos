package displaymgr

import (
	"cli/cmd/globals"
	"log"
)

func PrintRequest(reqJSON string) {
	if globals.IsJSONReq {
		log.Print(string(reqJSON))
	}
}
