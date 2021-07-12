package displaymgr

import (
	"cli/cmd/globals"
	"fmt"
)

func PrintRequest(reqJSON string) {
	if globals.IsJSONReq {
		fmt.Print(string(reqJSON))
	}
}
