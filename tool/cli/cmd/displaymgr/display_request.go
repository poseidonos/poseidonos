package displaymgr

import (
	"fmt"
)

func PrintRequest(reqJson string) {
	reqHeader := `{\"Request\":`
	reqFooter := `}`

	fmt.Println(reqHeader + reqJson + reqFooter)
}
