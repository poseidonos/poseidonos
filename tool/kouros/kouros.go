// Package kouros implements functions that can be used by clients developed in go to access the poseidonos
// and give commands to PoseidonOS and 2-Node HA
package kouros

import (
	"errors"
	//"kouros/ha"
	"kouros/pos"
)

// NewPOSManager creates a new manager object based on the type provided
// Currently only type "grpc" is supported
func NewPOSManager(managerType pos.POSInterface) (pos.POSManager, error) {
	switch managerType {
	case pos.GRPC:
		return &pos.POSGRPCManager{}, nil
	default:
		return nil, errors.New("Invalid POS Manager type")
	}
}

/*
// NewPOSManager creates a new manager object based on the type provided
// Currently only type "postgres" is supported
func NewHAManager(managerType ha.HAInterface) (ha.HAManager, error) {
	switch managerType {
	case ha.Postgres:
		return &ha.PostgresHAManager{}, nil
	default:
		return nil, errors.New("Invalid HA Manager type")
	}
}*/
