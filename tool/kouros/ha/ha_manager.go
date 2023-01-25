package ha

import (
	pb "kouros/api"
)

type HAInterface int64

const (
	Postgres HAInterface = iota
)

type HAManager interface {
	ListNodes() (*pb.ListNodeResponse, error)
}
