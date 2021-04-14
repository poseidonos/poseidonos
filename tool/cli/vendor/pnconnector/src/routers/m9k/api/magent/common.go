package magent

import (
	"github.com/influxdata/influxdb/client/v2"
)

func ExecuteQuery(query string) ([]client.Result, error) {
	var result []client.Result
	dbClient, err := IDBClient.ConnectDB()
	defer dbClient.Close()

	if err != nil {
		err = errConnInfluxDB
		return result, err
	}

	queryObject := client.Query{
		Command:   query,
		Database:  DBName,
		Precision: "ns",
	}

	if response, err := dbClient.Query(queryObject); err == nil {
		if response.Error() != nil {
			err = errQuery
		}
		result = response.Results

	} else {
		err = errQuery
		return result, err
	}
	return result, err
}
