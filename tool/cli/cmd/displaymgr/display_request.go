package displaymgr

import (
	"cli/cmd/globals"
	"fmt"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/reflect/protoreflect"
)

func PrintRequest(reqJson string) {
	if globals.IsJSONReq {
		reqHeader := `{"Request":`
		reqFooter := `}`

		fmt.Println(reqHeader + reqJson + reqFooter)
	}
}

func PrintProtoReqJson(req protoreflect.ProtoMessage) error {
	if globals.IsJSONReq {
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)

		if err != nil {
			return err
		}

		reqHeader := `{"Request":`
		reqFooter := `}`

		fmt.Println(reqHeader + string(reqJson) + reqFooter)
	}

	return nil
}
