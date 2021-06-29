package mocks

import (
	"context"
	"encoding/json"
	"errors"
	"github.com/influxdata/influxdb/client/v2"
	"github.com/influxdata/influxdb/models"
	"time"
)

type MockInfluxClient struct{}

type mockClient struct{}

func (m *mockClient) Ping(timeout time.Duration) (time.Duration, string, error) {
	now := time.Now()
	return time.Since(now), "v1", nil
}

func (m *mockClient) Write(bp client.BatchPoints) error {
	return nil
}

var VolumeQuery = "SELECT * from poseidon.default_rp.volumes where volid=0 order by time desc limit 1"

var VolumeQueryNoData = "SELECT * from poseidon.default_rp.volumes where volid=101 order by time desc limit 1"

var LastReadBandwidthQuery = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw", timestamp FROM "poseidon"."default_rp"."air" order by time desc limit 1`

var ArrayReadBandwidthQuery = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "bw", median(unixTimestamp) as timestamp FROM "poseidon"."default_rp"."air" WHERE time > now() - 15m and time > 0 GROUP BY time(1m) FILL(null)`

var AggregatedReadBandwidthQuery = `SELECT /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/, /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw" FROM "poseidon"."agg_rp"."mean_air" WHERE time > now() - 30d and time > 0 FILL(null)`

var LastReadIOPSQuery = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_read$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops", timestamp FROM "poseidon"."default_rp"."air" order by time desc limit 1`

var ArrayReadIOPSQuery = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_read$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "iops", median(unixTimestamp) as timestamp FROM "poseidon"."default_rp"."air" WHERE time > now() - 15m and time > 0 GROUP BY time(1m) FILL(null)`

var LastWriteBandwidthQuery = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_write$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw", timestamp FROM "poseidon"."default_rp"."air" order by time desc limit 1`

var ArrayWriteBandwidthQuery = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_write$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "bw", median(unixTimestamp) as timestamp FROM "poseidon"."default_rp"."air" WHERE time > now() - 15m and time > 0 GROUP BY time(1m) FILL(null)`

var LastWriteIOPSQuery = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_write$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops", timestamp FROM "poseidon"."default_rp"."air" order by time desc limit 1`

var ArrayWriteIOPSQuery = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_write$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "iops", median(unixTimestamp) as timestamp FROM "poseidon"."default_rp"."air" WHERE time > now() - 15m and time > 0 GROUP BY time(1m) FILL(null)`

var LatencyQuery = `SELECT mean(/^lat_data_[\S]+_aid_arr_[\S]+_sid_arr_0_mean$/), mean(/^lat_data_[\S]+_aid_arr_[\S]+_aid$/) as "latency", median(unixTimestamp) as timestamp FROM "poseidon"."default_rp"."air" WHERE time > now() - 15m and time > 0 GROUP BY time(1m) FILL(null)`

var LastLatencyQuery = `SELECT /^lat_data_[\S]+_aid_arr_[\S]+_sid_arr_0_mean$/, /^lat_data_[\S]+_aid_arr_[\S]+_aid$/ as "latency", timestamp FROM "poseidon"."default_rp"."air" order by time desc limit 1`

var MemoryLastQuery = "SELECT last(used_percent) AS mean_used_percent FROM poseidon.default_rp.mem LIMIT 1"

var MemoryDefaultQuery = "SELECT mean(used_percent) AS mean_used_percent FROM poseidon.default_rp.mem WHERE time > now() - 15m GROUP BY time(1m)"

var MemoryNoDataQuery = "SELECT mean(used_percent) AS mean_used_percent FROM poseidon.default_rp.mem WHERE time > now() - 60d GROUP BY time(60d)"

var CPULastQuery = "SELECT last(usage_user) AS mean_usage_user FROM poseidon.default_rp.cpu LIMIT 1"

var CPUDefaultQuery = "SELECT mean(usage_user) AS mean_usage_user FROM poseidon.default_rp.cpu WHERE time > now() - 15m GROUP BY time(1m)"

var CPUNoDataQuery = "SELECT mean(usage_user) AS mean_usage_user FROM poseidon.default_rp.cpu WHERE time > now() - 60d GROUP BY time(60d)"

var DiskLastQuery = "SELECT last(used) AS mean_used_bytes FROM poseidon.default_rp.disk LIMIT 1"

var DiskDefaultQuery = "SELECT mean(used) AS mean_used_bytes FROM poseidon.default_rp.disk WHERE time > now() - 15m GROUP BY time(1m)"

var DiskNoDataQuery = "SELECT mean(used) AS mean_used_bytes FROM poseidon.default_rp.disk WHERE time > now() - 60d GROUP BY time(60d)"

var NetLastQuery = "SELECT last(bytes_recv), last(bytes_sent), last(drop_in), last(drop_out), last(err_in), last(err_out), last(packets_recv), last(packets_sent)  FROM poseidon.default_rp.net LIMIT 1"

var NetDefaultQuery = "SELECT mean(bytes_recv) ,mean(bytes_sent), mean(drop_in), mean(drop_out), mean(err_in), mean(err_out), mean(packets_recv), mean(packets_sent)  FROM poseidon.default_rp.net WHERE time > now() - 15m GROUP BY time(1m)"

var NetNoDataQuery = "SELECT mean(bytes_recv) ,mean(bytes_sent), mean(drop_in), mean(drop_out), mean(err_in), mean(err_out), mean(packets_recv), mean(packets_sent)  FROM poseidon.default_rp.net WHERE time > now() - 60d GROUP BY time(60d)"

var NetDriverQuery = `SELECT time, "name", driver FROM poseidon.autogen.ethernet`

var NetDriverNoDataQ = `SELECT time, "name", driver FROM poseidonDataNil.autogen.ethernet`

var NetAddressQuery = `SELECT time, "name", address FROM poseidon.autogen.ethernet`

var NetAddressNoDataQ = `SELECT time, "name", address FROM poseidonNoData.autogen.ethernet`

var RebuildLogQ = `SELECT "value" FROM "poseidon"."autogen"."rebuilding_status" WHERE time > now() - 1h`

var RebuildLogNoDataQ = `SELECT "value" FROM "poseidon"."autogen"."rebuilding_status" WHERE time > now() - 1d`

func (m *mockClient) Query(q client.Query) (*client.Response, error) {
	//series := models.Row{}
	message := client.Message{
		Level: "level1",
		Text:  "Message",
	}
	messages := []*client.Message{}
	messages = append(messages, &message)
	rows := []models.Row{}
	switch q.Command {
	case VolumeQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"volid",
			},
			Values: [][]interface{}{
				{json.Number("0"), json.Number(0)},
			},
		}
		rows = append(rows, row)
	case VolumeQueryNoData:
		row := models.Row{
			Columns: []string{},
			Values:  [][]interface{}{},
		}
		rows = append(rows, row)

	case LastReadBandwidthQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"perf_data_0_tid_arr_0_aid_arr_0_bw_read",
				"perf_data_0_tid_arr_0_aid_arr_0_aid",
				"perf_data_0_tid_arr_1_aid_arr_0_bw_read",
				"perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
			},
		}
		rows = append(rows, row)
	case ArrayReadBandwidthQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_bw_read",
				"bw_perf_data_0_tid_arr_0_aid_arr_0_aid",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_bw_read",
				"bw_perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case AggregatedReadBandwidthQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_bw_read",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_aid",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_bw_read",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case LastWriteBandwidthQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"perf_data_0_tid_arr_0_aid_arr_0_bw_write",
				"perf_data_0_tid_arr_0_aid_arr_0_aid",
				"perf_data_0_tid_arr_1_aid_arr_0_bw_write",
				"perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
			},
		}
		rows = append(rows, row)
	case ArrayWriteBandwidthQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_bw_write",
				"bw_perf_data_0_tid_arr_0_aid_arr_0_aid",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_bw_write",
				"bw_perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case LastReadIOPSQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"perf_data_0_tid_arr_0_aid_arr_0_iops_read",
				"perf_data_0_tid_arr_0_aid_arr_0_aid",
				"perf_data_0_tid_arr_1_aid_arr_0_iops_read",
				"perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
			},
		}
		rows = append(rows, row)
	case ArrayReadIOPSQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_iops_read",
				"iops_perf_data_0_tid_arr_0_aid_arr_0_aid",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_iops_read",
				"iops_perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case LastWriteIOPSQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"perf_data_0_tid_arr_0_aid_arr_0_iops_write",
				"perf_data_0_tid_arr_0_aid_arr_0_aid",
				"perf_data_0_tid_arr_1_aid_arr_0_iops_write",
				"perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
			},
		}
		rows = append(rows, row)
	case ArrayWriteIOPSQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_perf_data_0_tid_arr_0_aid_arr_0_iops_write",
				"iops_perf_data_0_tid_arr_0_aid_arr_0_aid",
				"mean_perf_data_0_tid_arr_1_aid_arr_0_iops_write",
				"iops_perf_data_0_tid_arr_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case LastLatencyQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"lat_data_0_aid_arr_0_sid_arr_0_mean",
				"latency_lat_data_0_aid_arr_0_aid",
				"lat_data_0_aid_arr_0_sid_arr_0_mean",
				"latency_lat_data_0_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
			},
		}
		rows = append(rows, row)
	case LatencyQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_lat_data_0_aid_arr_0_sid_arr_0_mean",
				"latency_lat_data_0_aid_arr_0_aid",
				"mean_lat_data_1_aid_arr_0_sid_arr_0_mean",
				"latency_lat_data_1_aid_arr_0_aid",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100"), json.Number("0"), json.Number("200"), json.Number("1")},
				{json.Number("1589872050483870738"), json.Number("400"), json.Number("0"), json.Number("300"), json.Number("0")},
			},
		}
		rows = append(rows, row)
	case MemoryLastQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_used_percent",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("70")},
			},
		}
		rows = append(rows, row)
	case MemoryDefaultQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_used_percent",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("50")},
				{json.Number("1589872050483870738"), json.Number("40")},
			},
		}
		rows = append(rows, row)
	case CPULastQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_usage_user",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("20")},
			},
		}
		rows = append(rows, row)
	case CPUDefaultQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_usage_user",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("20")},
				{json.Number("1589872050483870738"), json.Number("30")},
			},
		}
		rows = append(rows, row)
	case DiskLastQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_used_bytes",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("200000")},
			},
		}
		rows = append(rows, row)
	case DiskDefaultQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_used_bytes",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), json.Number("100000")},
				{json.Number("1589872050483870738"), json.Number("200000")},
			},
		}
		rows = append(rows, row)
	case NetLastQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_bytes_recv",
				"mean_bytes_sent",
				"mean_drop_in",
				"mean_drop_out",
				"mean_err_in",
				"mean_err_out",
				"mean_packets_recv",
				"mean_packets_sent",
			},
			Values: [][]interface{}{
				{
					json.Number("1589872050483860738"),
					json.Number("10"),
					json.Number("20"),
					json.Number("30"),
					json.Number("40"),
					json.Number("50"),
					json.Number("60"),
					json.Number("70"),
					json.Number("80"),
				},
			},
		}
		rows = append(rows, row)
	case NetDefaultQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"mean_bytes_recv",
				"mean_bytes_sent",
				"mean_drop_in",
				"mean_drop_out",
				"mean_err_in",
				"mean_err_out",
				"mean_packets_recv",
				"mean_packets_sent",
			},
			Values: [][]interface{}{
				{
					json.Number("1589872050483860738"),
					json.Number("10"),
					json.Number("20"),
					json.Number("30"),
					json.Number("40"),
					json.Number("50"),
					json.Number("60"),
					json.Number("70"),
					json.Number("80"),
				},
				{
					json.Number("1589872050483870738"),
					json.Number("50"),
					json.Number("60"),
					json.Number("30"),
					json.Number("40"),
					json.Number("90"),
					json.Number("60"),
					json.Number("70"),
					json.Number("80"),
				},
			},
		}
		rows = append(rows, row)
	case NetDriverQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"name",
				"driver",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), "interface", "driver"},
			},
		}
		rows = append(rows, row)
	case NetAddressQuery:
		row := models.Row{
			Columns: []string{
				"time",
				"name",
				"address",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), "interface", "address"},
			},
		}
		rows = append(rows, row)
	case RebuildLogQ:
		row := models.Row{
			Columns: []string{
				"time",
				"value",
			},
			Values: [][]interface{}{
				{json.Number("1589872050483860738"), "Log line 1"},
			},
		}
		rows = append(rows, row)
	case CPUNoDataQuery, DiskNoDataQuery, MemoryNoDataQuery,
		NetNoDataQuery, NetDriverNoDataQ, NetAddressNoDataQ, RebuildLogNoDataQ:
	default:
		return nil, errors.New("could not query to database")
	}
	result := client.Result{
		Series:   rows,
		Messages: messages,
	}
	results := []client.Result{}
	results = append(results, result)
	response := client.Response{
		Results: results,
	}
	return &response, nil
}

func (m *mockClient) Close() error {
	return nil
}

func (m *mockClient) QueryCtx(ctx context.Context, q client.Query) (*client.Response, error) {
	var response client.Response
	return &response, nil
}

// QueryAsChunk makes an InfluxDB Query on the database. This will fail if using
// the UDP client.
func (m *mockClient) QueryAsChunk(q client.Query) (*client.ChunkedResponse, error) {
	var response client.ChunkedResponse
	return &response, nil
}

func (m MockInfluxClient) ConnectDB() (client.Client, error) {
	return &mockClient{}, nil
}

/*
func main() {
	c := &mockClient{}
	res, err := c.Query(client.Query{})
	if err == nil {
		fmt.Println(res)
	}
}
*/
