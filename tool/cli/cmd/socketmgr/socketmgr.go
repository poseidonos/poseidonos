package socketmgr

import (
	"bufio"
	"cli/cmd/globals"
	"fmt"
	"io"
	"net"
	"os"
	"pnconnector/src/log"
)

var conn net.Conn

func connectToCliServer() {
	var err error
	conn, err = net.Dial("tcp", globals.IPv4+":"+globals.Port)
	if err != nil {
		fmt.Fprintf(os.Stderr, "cannot connect to the cli server: %s\n", err)
		log.Error("cannot connect to the cli server:", err)
		return
	}
}

func SendReqAndReceiveRes(reqJSON string) string {
	connectToCliServer()
	defer closeConn()

	if conn == nil {
		log.Error("cannot send a request to cli server: not connected")
		return ""
	}

	fmt.Fprintf(conn, reqJSON)
	res, err := bufio.NewReader(conn).ReadString('\n')
	if err != nil && err != io.EOF {
		log.Error("could not receive data from the cli sever:", err)
		return ""
	}

	return res
}

func closeConn() {
	if conn == nil {
		return
	}
	conn.Close()
}
