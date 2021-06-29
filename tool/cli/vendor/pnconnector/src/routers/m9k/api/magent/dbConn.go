package magent

import (
	"github.com/influxdata/influxdb/client/v2"
)

// InfluxDBClient contains the function to connect to influxDB
type InfluxDBClient interface {
	ConnectDB() (client.Client, error)
}

// InfluxClient implements InfluxDBClient
type InfluxClient struct{}

// ConnectDB creates an influxdb client and returns it
func (i InfluxClient) ConnectDB() (client.Client, error) {
	client, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: DBAddress,
	})
	return client, err
}

// IDBClient should be used for connecting to InfluxDB
var IDBClient InfluxDBClient = InfluxClient{}
