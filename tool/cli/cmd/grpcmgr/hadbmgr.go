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
const listReplicationQuery = `SELECT "id","source_volume_id","source_wal_volume_id","destination_volume_id","destination_wal_volume_id" FROM "replication"`
const getIpv4Query = `SELECT "ip" FROM "node" WHERE "name"='%s'`

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

func SendListHaReplication(req *pb.ListHaReplicationRequest) (*pb.ListHaReplicationResponse, error) {

	db, err := OpenHaDb()

	// close database
	defer db.Close()
	// check db
	err = db.Ping()
	if err != nil {
		return nil, err
	}

	rows, err := db.Query(listReplicationQuery)
	if err != nil {
		return nil, err
	}

	var (
		code        int32  = 0
		description string = "SUCCESS"
	)
	status := &pb.Status{Code: &code, Description: &description}
	result := &pb.ListHaReplicationResponse_Result{Status: status}

	defer rows.Close()
	for rows.Next() {
		var (
			id                     int32
			sourceVolumeId         int32
			sourceWalVolumeId      int32
			destinationVolumeId    int32
			destinationWalVolumeId int32
		)

		err = rows.Scan(&id, &sourceVolumeId, &sourceWalVolumeId, &destinationVolumeId, &destinationWalVolumeId)
		if err != nil {
			return nil, err
		}
		result.Data = append(result.Data,
			&pb.ListHaReplicationResponse_Result_Replication{Id: id, SourceVolumeId: sourceVolumeId,
				SourceWalVolumeId: sourceWalVolumeId, DestinationVolumeId: destinationVolumeId,
				DestinationWalVolumeId: destinationWalVolumeId})
	}

	res := &pb.ListHaReplicationResponse{Command: req.Command, Rid: req.Rid, Result: result}

	return res, nil
}

func SendStartHaReplication(req *pb.StartHaReplicationRequest) (*pb.StartHaReplicationResponse, error) {

	db, err := OpenHaDb()

	// close database
	defer db.Close()
	// check db
	err = db.Ping()
	if err != nil {
		return nil, err
	}

	primaryNodeName := req.Param.PrimaryNodeName
	primaryArrayName := req.Param.PrimaryArrayName
	primaryVolumeName := req.Param.PrimaryVolumeName
	primaryWalVolumeName := req.Param.PrimaryWalVolumeName
	secondaryNodeName := req.Param.SecondaryNodeName
	secondaryArrayName := req.Param.SecondaryArrayName
	secondaryVolumeName := req.Param.SecondaryVolumeName
	secondaryWalVolumeName := req.Param.SecondaryWalVolumeName
	timestamp := req.Param.Timestamp

	// mj: declare query here for this command as the query
	startHaRepPrimaryQuery := `INSERT INTO command ("node_name","content","status","timestamp") ` +
		`VALUES ` + `('` + primaryNodeName + `', 'volumecopy ` +
		primaryNodeName + ` ` + primaryArrayName + ` ` + primaryVolumeName + ` ` + primaryWalVolumeName +
		secondaryNodeName + ` ` + secondaryArrayName + ` ` + secondaryVolumeName + ` ` + secondaryWalVolumeName +
		`', 'Created', '` + timestamp + `')`

	startHaRepSecondaryQuery := `INSERT INTO command ("node_name","content","status","timestamp") ` +
		`VALUES ` + `('` + secondaryNodeName + `', 'volumecopy ` +
		primaryNodeName + ` ` + primaryArrayName + ` ` + primaryVolumeName + ` ` + primaryWalVolumeName +
		secondaryNodeName + ` ` + secondaryArrayName + ` ` + secondaryVolumeName + ` ` + secondaryWalVolumeName +
		`', 'Created', '` + timestamp + `')`

	rows, err := db.Query(startHaRepPrimaryQuery)
	if err != nil {
		return nil, err
	}

	rows, err = db.Query(startHaRepSecondaryQuery)
	if err != nil {
		return nil, err
	}

	var (
		code        int32  = 0
		description string = "SUCCESS"
	)
	status := &pb.Status{Code: &code, Description: &description}
	result := &pb.StartHaReplicationResponse_Result{Status: status}

	defer rows.Close()

	res := &pb.StartHaReplicationResponse{Command: req.Command, Rid: req.Rid, Result: result}

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

func GetIpv4(nodeName string) (string, error) {
	db, err := OpenHaDb()
	if err != nil {
		return "", errors.New("an error occured while opening ha database: " + err.Error())
	}

	// close database
	defer db.Close()
	// check db
	err = db.Ping()
	if err != nil {
		return "", errors.New("an error occured while ping to ha database: " + err.Error())
	}

	query := fmt.Sprintf(getIpv4Query, nodeName)
	rows, err := db.Query(query)
	if err != nil {
		return "", errors.New("an error occured while query to ha database: " + err.Error())
	}

	defer rows.Close()

	var (
		NodegRpcIp   string = ""
		NodegRpcPort string = globals.GrpcPort
	)

	for rows.Next() {
		err = rows.Scan(&NodegRpcIp)
		if err != nil {
			return "", errors.New("an error occured while scanning ha database: " + err.Error())
		}
	}

	gRpcAddress := fmt.Sprintf("%s:%s", NodegRpcIp, NodegRpcPort)

	return gRpcAddress, nil
}
