package displaymgr

import (
	"cli/cmd/globals"
	"fmt"
)

func PrintRequest(reqJson string) {
	if globals.IsJSONReq {
		reqHeader := `{"Request":`
		reqFooter := `}`

		fmt.Println(reqHeader + reqJson + reqFooter)
	}
}
