package grpcmgr

import (
	pb "cli/api"
	"cli/cmd/globals"
	sql "database/sql"
	"errors"
	"fmt"
	"os"
)

const listNodeQuery = `SELECT "name","ip","lastseen" FROM "node"`
const listVolumeQuery = `SELECT "id","name","node_name","array_name","size","lastseen" FROM "volume"`

func SendListNode(req *pb.ListNodeRequest) (*pb.ListNodeResponse, error) {

	db, err := OpenHaDb()

	// close database
	defer db.Close()
	// check db
	err = db.Ping()
	if err != nil {
		return nil, err
	}

	rows, err := db.Query(listNodeQuery)
	if err != nil {
		return nil, err
	}

	var (
		code        int32  = 0
		description string = "SUCCESS"
	)
	status := &pb.Status{Code: &code, Description: &description}
	result := &pb.ListNodeResponse_Result{Status: status}

	defer rows.Close()
	for rows.Next() {
		var name, ip, lastSeen string
		err = rows.Scan(&name, &ip, &lastSeen)
		if err != nil {
			return nil, err
		}
		result.Data = append(result.Data, &pb.ListNodeResponse_Result_Node{Name: name, Ip: ip, Lastseen: lastSeen})
	}

	res := &pb.ListNodeResponse{Command: req.Command, Rid: req.Rid, Result: result}

	return res, nil
}

func SendListHaVolume(req *pb.ListHaVolumeRequest) (*pb.ListHaVolumeResponse, error) {

	db, err := OpenHaDb()

	// close database
	defer db.Close()
	// check db
	err = db.Ping()
	if err != nil {
		return nil, err
	}

	rows, err := db.Query(listVolumeQuery)
	if err != nil {
		return nil, err
	}

	var (
		code        int32  = 0
		description string = "SUCCESS"
	)
	status := &pb.Status{Code: &code, Description: &description}
	result := &pb.ListHaVolumeResponse_Result{Status: status}

	defer rows.Close()
	for rows.Next() {
		var (
			id                                  int32
			size                                int64
			name, nodeName, arrayName, lastSeen string
		)

		err = rows.Scan(&id, &name, &nodeName, &arrayName, &size, &lastSeen)
		if err != nil {
			return nil, err
		}
		result.Data = append(result.Data,
			&pb.ListHaVolumeResponse_Result_Volume{Id: id, Name: name, NodeName: nodeName,
				ArrayName: arrayName, Size: size, Lastseen: lastSeen})
	}

	res := &pb.ListHaVolumeResponse{Command: req.Command, Rid: req.Rid, Result: result}

	return res, nil
}

func OpenHaDb() (*sql.DB, error) {
	host, port, username, password, dbname, err := GetHaDbInfo()
	if err != nil {
		fmt.Println(err)
	}

	psqlconn := fmt.Sprintf("host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
		host, port, username, password, dbname)

	// open database
	db, err := sql.Open("postgres", psqlconn)
	if err != nil {
		return nil, err
	}

	return db, err
}

func GetHaDbInfo() (string, string, string, string, string, error) {

	var host, port, username, password, dbname string
	var exists bool

	host, exists = os.LookupEnv(globals.HaDbIPVar)
	if !exists {
		return "", "", "", "", "", errors.New("You must set the IP address as a environmental variable: " + globals.HaDbIPVar)
	}

	port, exists = os.LookupEnv(globals.HaDbPortVar)
	if !exists {
		return "", "", "", "", "", errors.New("You must set the port number as a environmental variable: " + globals.HaDbPortVar)
	}

	username, exists = os.LookupEnv(globals.HaDbUserVar)
	if !exists {
		return "", "", "", "", "", errors.New("You must set the username as a environmental variable: " + globals.HaDbNameVar)
	}

	password, exists = os.LookupEnv(globals.HaDbPasswordVar)
	if !exists {
		return "", "", "", "", "", errors.New("You must set the password as a environmental variable: " + globals.HaDbPasswordVar)
	}

	dbname, exists = os.LookupEnv(globals.HaDbNameVar)
	if !exists {
		return "", "", "", "", "", errors.New("You must set the database name as a environmental variable: " + globals.HaDbNameVar)
	}

	return host, port, username, password, dbname, nil
}
