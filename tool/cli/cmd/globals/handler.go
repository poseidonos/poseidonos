package globals

import (
	"fmt"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

func PrintErrMsg(err error) {
	status, _ := status.FromError(err)
	switch status.Code() {
	case codes.ResourceExhausted:
		fmt.Println(PosBusy)
	}
}
