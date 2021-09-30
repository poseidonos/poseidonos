package socketmgr

import (
	"bufio"
	"cli/cmd/globals"
	"errors"
	"fmt"
	"net"
	"pnconnector/src/log"
)

var conn net.Conn

func Connect() {
	// Connect to server
	var err error
	conn, err = net.Dial("tcp", globals.IPv4+":"+globals.Port)
	if err != nil {
		log.Error("error:", err)
	}
}

func SendReqAndReceiveRes(reqJSON string) (string, error) {
	if conn == nil {
		println("Error: cannot connect to the PoseidonOS server!")
		return "", errors.New("SocketMgr: not connected to the server")
	}

	fmt.Fprintf(conn, reqJSON)
	res, err := bufio.NewReader(conn).ReadString('\n')
	if err != nil {
		log.Debug("error:", err)
	}

	return res, nil
}

func Close() {
	if conn == nil {
		return
	}
	conn.Close()
}
