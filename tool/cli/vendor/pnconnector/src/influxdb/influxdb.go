package influxdb

import (
	"pnconnector/src/errors"
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/api/magent"
	"pnconnector/src/routers/m9k/model"
	"encoding/json"
	"github.com/google/uuid"
	"github.com/influxdata/influxdb/client/v2"
	"time"
)

var ip = "127.0.0.1"

var config = client.HTTPConfig{
	Addr: "http://" + ip + ":8086",
}

var POS_BUSY_CODE = 1030
var VOLUMES_MEASUREMENT = "volumes"

var (
	CreateErr = errors.New("Influx Create fail")
	DeleteErr = errors.New("Influx Delete fail")
)

// getVolumeId gets the newly created Volume Id from the Volume List
// Currently, both Delete and Create Volume APIs does not take ID as a parameter
// getVolumeId is called after volume is created. It queries the volume list and finds the volume Id from volume list
// returns -1 if the volume is not present in the list
func getVolumeId(listVolume func(string, interface{}) (model.Request, model.Response, error), name string) int {
	listXrid, err := uuid.NewUUID()
	if err != nil {
		return -1
	}
	req := model.Request{}
	count := 0
	_, listRes, listErr := listVolume(listXrid.String(), req.Param)

	// Observed the "POS is busy" response especially in case of multiple volume creation
	// So, a retry logic is applied to retry listing the volumes
	for count < 10 {
		if listRes.Result.Status.Code == POS_BUSY_CODE {
			time.Sleep(5 * time.Second)

			_, listRes, listErr = listVolume(listXrid.String(), req.Param)
			count++
		} else {
			break
		}
	}

	if listErr != nil || listRes.Result.Data == nil {
		return -1
	}

	volumes := listRes.Result.Data.(map[string]interface{})["volumes"].([]interface{})
	for i := 0; i < len(volumes); i++ {
		if volumes[i].(map[string]interface{})["name"].(string) == name {
			volId, err := volumes[i].(map[string]interface{})["id"].(json.Number).Int64()
			if err != nil {
				return -1
			}
			return int(volId)
		}
	}
	return -1

}

// CreateVolume is used to mark the volume creation time in InfluxDB. This time is used in AIR queries,
// so that we dont get stale data in volume level performance queries
// This function is expected to be executed as ago routine after we get the response for a volume Creation
func CreateVolume(listVolume func(string, interface{}) (model.Request, model.Response, error), param interface{}, response model.Response) {

	// This function will make the application recover from any panic raised while executing the current function
	defer func() {
		if err := recover(); err != nil {
			log.Info("Panic occured while writing to InfluxDB")
			log.Info(err)
		}
	}()

	// If Volume creation is not successful, return
	if response.Result.Status.Code != 0 {
		return
	}

	name := ""

	// In case of Multivolume creation, the Param will be of type model.VolumeParam
	// In case of Single Volume creation, the param will be of type map[string]interface{}
	// The below switch is for extracting the volume name from the param depending on the case
	switch param.(type) {
	case map[string]interface{}:
		name = param.(map[string]interface{})["name"].(string)
	case model.VolumeParam:
		name = param.(model.VolumeParam).Name
	}

	// Get Volume Id from Name
	id := getVolumeId(listVolume, name)
	if id == -1 {
		return
	}

	// Creating an InfluxDB http client
	httpClient, err := client.NewHTTPClient(config)
	if err != nil {
		log.Info(err)
		return
	}

	//Setting the fields to be inserted
	fields := map[string]interface{}{
		"volid": id,
	}
	tags := map[string]string{}

	// Create a point object with the tags and fields. A point corresponds to  a line in InfluxDB
	point, err := client.NewPoint(VOLUMES_MEASUREMENT, tags, fields, time.Now())
	if err != nil {
		log.Info(err)
		return
	}

	// Create a batchpoint object. points are sent to influxdb in batches
	batchPoint, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:        magent.DBName,
		RetentionPolicy: magent.DefaultRP,
	})
	if err != nil {
		log.Info(err)
		return
	}

	batchPoint.AddPoint(point)

	// write the batch to InfluxDB
	err = httpClient.Write(batchPoint)
	if err != nil {
		log.Info(err)
	}
}

func DeleteVolume() error {
	log.Info("Delete Volume !!!!")

	//return DeleteErr
	return nil
}
