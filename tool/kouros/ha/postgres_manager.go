package ha

import (
	"database/sql"
	"errors"
	"fmt"
	pb "kouros/api"
	"kouros/ha/postgres"
	"kouros/utils"

	"google.golang.org/protobuf/encoding/protojson"
)

type PostgresHAManager struct {
	connection postgres.PostgresConnection
	requestor  string
}

func (p *PostgresHAManager) Init(client string, config interface{}) error {
	haConfig, ok := config.(map[string]string)
	if !ok {
		return errors.New("Init method expects the config of type map[string]string")
	}

	host, exists := haConfig["host"]
	if !exists {
		return errors.New("You must provide the host parameter for Kouros connection")
	}

	port, exists := haConfig["port"]
	if !exists {
		return errors.New("You must provide the port parameter for Kouros connection")
	}

	username, exists := haConfig["username"]
	if !exists {
		return errors.New("You must provide the username parameter for Kouros connection")
	}

	password, exists := haConfig["password"]
	if !exists {
		return errors.New("You must provide the password parameter for Kouros connection")
	}

	dbname, exists := haConfig["dbname"]
	if !exists {
		return errors.New("You must provide the dbname parameter for Kouros connection")
	}
	p.connection = postgres.PostgresConnection{
		Host:     host,
		Port:     port,
		Username: username,
		Password: password,
		DBName:   dbname,
	}
	p.requestor = client
	return nil
}

func (p *PostgresHAManager) connect() (*sql.DB, error) {
	psqlconn := fmt.Sprintf("host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
		p.connection.Host, p.connection.Port, p.connection.Username, p.connection.Password, p.connection.DBName)
	return sql.Open("postgres", psqlconn)
}

func (p *PostgresHAManager) ListNodes() ([]byte, error) {
	command := "LISTNODE"
	uuid := utils.GenerateUUID()
	req := &pb.ListNodeRequest{Command: command, Rid: uuid, Requestor: p.requestor}
	res, err := postgres.SendListNode(p.connection, req)
	if err != nil {
		return nil, err
	}
	return protojson.Marshal(res)
}

func (p *PostgresHAManager) ListHAReplication() ([]byte, error) {
	command := "LISTHAREPLICATION"
	uuid := utils.GenerateUUID()
	req := &pb.ListHaReplicationRequest{Command: command, Rid: uuid, Requestor: p.requestor}
	res, err := postgres.SendListHaReplication(p.connection, req)
	if err != nil {
		return nil, err
	}
	return protojson.Marshal(res)
}

func (p *PostgresHAManager) ListHAVolumes() ([]byte, error) {
	command := "LISTHAVOLUME"
	uuid := utils.GenerateUUID()
	req := &pb.ListHaVolumeRequest{Command: command, Rid: uuid, Requestor: p.requestor}
	res, err := postgres.SendListHaVolume(p.connection, req)
	if err != nil {
		return nil, err
	}
	return protojson.Marshal(res)
}

func (p *PostgresHAManager) StartHAReplication(param []byte) ([]byte, error) {
	var startHAReplicationParam pb.StartHaReplicationRequest_Param
	command := "STARTHAREPLICATION"
	uuid := utils.GenerateUUID()
	protojson.Unmarshal(param, &startHAReplicationParam)
	req := &pb.StartHaReplicationRequest{Command: command, Rid: uuid, Requestor: p.requestor}
	res, err := postgres.SendStartHaReplication(p.connection, req)
	if err != nil {
		return nil, err
	}
	return protojson.Marshal(res)
}
