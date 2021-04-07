// +build !sslon

package handler

import (
	"net"
)

//var conn net.Conn

func Dial(network, addr string) (net.Conn, error) {
	//var err error = nil
	conn, err := net.Dial(network, addr)
	return conn, err
}
