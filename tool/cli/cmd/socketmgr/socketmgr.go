package socketmgr

import (
	"bufio"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"pnconnector/src/log"
)

const (
	connErrEventName = "POS_CONNECTION_ERROR"
	connErrCause     = "Cannot connect to PoseidonOS. It might not be running."
	connErrSolution  = "Launch PoseidonOS first ($YOURPOSDIR/bin/poseidonos-cli system start)."
)

func connectToCliServer() (net.Conn, error) {
	var err error
	conn, err := net.Dial("tcp", globals.IPv4+":"+globals.Port)
	if err != nil {
		log.Error("cannot connect to the cli server:", err)
	}

	return conn, err
}

func SendReqAndReceiveRes(reqJSON string) string {
	conn, err := connectToCliServer()
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return buildConnErrResp(reqJSON, err.Error())
	}
	defer conn.Close()

	fmt.Fprintf(conn, reqJSON)
	res, err := bufio.NewReader(conn).ReadString('\n')
	if err != nil && err != io.EOF {
		log.Error("could not receive data from the cli sever:", err)
		return buildConnErrResp(reqJSON, err.Error())
	}

	return res
}

func buildConnErrResp(req string, errMsg string) string {
	reqJson := messages.Request{}
	json.Unmarshal([]byte(req), &reqJson)

	resJson := messages.Response{}
	resJson.RID = reqJson.RID
	resJson.COMMAND = reqJson.COMMAND
	resJson.RESULT.STATUS.CODE = globals.CliServerFailCode
	resJson.RESULT.STATUS.EVENTNAME = connErrEventName
	resJson.RESULT.STATUS.DESCRIPTION = errMsg
	resJson.RESULT.STATUS.CAUSE = connErrCause
	resJson.RESULT.STATUS.SOLUTION = connErrSolution

	resJsonStr, _ := json.Marshal(resJson)

	return string(resJsonStr)
}
