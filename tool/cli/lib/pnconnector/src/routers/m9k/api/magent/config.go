package magent

import "pnconnector/src/errors"

var DBName = "poseidon"
var DBAddress = "http://127.0.0.1:8086"
var AggTime = []string{"7d", "30d"}
var AggRP = "agg_rp"
var DefaultRP = "default_rp"
var TimeGroupsDefault = map[string]string{
	"1m":  "1s",
	"5m":  "1s",
	"15m": "1m",
	"1h":  "1m",
	"6h":  "1m",
	"12h": "5m",
	"24h": "10m",
	"7d":  "1h",
	"30d": "6h",
	"60d": "60d",
}

var (
	LatencyField   = "sid_arr_0_mean"
	BWReadField    = "bw_read"
	BWWriteField   = "bw_write"
	IOPSReadField  = "iops_read"
	IOPSWriteField = "iops_write"
	MeanFieldKey   = "mean"
	PerfFieldKey   = "perf"
	LatFieldKey    = "lat_"
)

var (
	errConnInfluxDB = errors.New("could not connect to Influxdb")
	errQuery        = errors.New("could not query to database")
	errQueryCode    = 21000
	errData         = errors.New("no data available")
	errDataCode     = 20313
	errEndPoint     = errors.New("use time from 1m,5m,15m,1h,6h,12h,24h,7d,30d")
	errEndPointCode = 21010
)

var VolumeQuery = "SELECT * from %s.%s.volumes where volid=%s order by time desc limit 1"

var netAggRPQ = "SELECT bytes_recv AS mean_bytes_recv , bytes_sent AS mean_bytes_sent, drop_in AS mean_drop_in, drop_out AS mean_drop_out, err_in AS mean_err_in, err_out AS mean_err_out, packets_recv AS mean_packets_recv, packets_sent AS mean_packets_sent  FROM %s.%s.mean_net WHERE time > now() - %s"
var netDefaultRPQ = "SELECT mean(bytes_recv) ,mean(bytes_sent), mean(drop_in), mean(drop_out), mean(err_in), mean(err_out), mean(packets_recv), mean(packets_sent)  FROM %s.%s.net WHERE time > now() - %s GROUP BY time(%s)"
var netLastRecordQ = "SELECT last(bytes_recv), last(bytes_sent), last(drop_in), last(drop_out), last(err_in), last(err_out), last(packets_recv), last(packets_sent)  FROM %s.%s.net LIMIT 1"
var netAddQ = "SELECT time, \"name\", address FROM %s.autogen.ethernet"
var netDriverQ = "SELECT time, \"name\", driver FROM %s.autogen.ethernet"
var cpuAggRPQ = "SELECT usage_user AS mean_usage_user FROM %s.%s.mean_cpu WHERE time > now() - %s"
var cpuDefaultRPQ = "SELECT mean(usage_user) AS mean_usage_user FROM %s.%s.cpu WHERE time > now() - %s GROUP BY time(%s)"
var cpuLastRecordQ = "SELECT last(usage_user) AS mean_usage_user FROM %s.%s.cpu LIMIT 1"
var diskAggRPQ = "SELECT used AS mean_used_bytes FROM %s.%s.mean_disk WHERE time > now() - %s"
var diskDefaultRPQ = "SELECT mean(used) AS mean_used_bytes FROM %s.%s.disk WHERE time > now() - %s GROUP BY time(%s)"
var diskLastRecordQ = "SELECT last(used) AS mean_used_bytes FROM %s.%s.disk LIMIT 1"
var memoryAggRPQ = "SELECT used_percent AS mean_used_percent FROM %s.%s.mean_mem WHERE time > now() - %s"
var memoryDefaultRPQ = "SELECT mean(used_percent) AS mean_used_percent FROM %s.%s.mem WHERE time > now() - %s GROUP BY time(%s)"
var memoryLastRecordQ = "SELECT last(used_percent) AS mean_used_percent FROM %s.%s.mem LIMIT 1"

var ReadBandwidthAggRPQ = `SELECT /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/, /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw" FROM "%s"."%s"."mean_air" WHERE time > now() - %s and time > %s FILL(null)`
var ReadBandwidthDefaultRPQ = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "bw", median(unixTimestamp) as timestamp FROM "%s"."%s"."air" WHERE time > now() - %s and time > %s GROUP BY time(%s) FILL(null)`
var ReadBandwidthLastRecordQ = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_read$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw", timestamp FROM "%s"."%s"."air" order by time desc limit 1`

var WriteBandwidthAggRPQ = `SELECT /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_write$/, /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw" FROM "%s"."%s"."mean_air" WHERE time > now() - %s and time > %s FILL(null)`
var WriteBandwidthDefaultRPQ = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_write$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "bw", median(unixTimestamp) as timestamp FROM "%s"."%s"."air" WHERE time > now() - %s and time > %s GROUP BY time(%s) FILL(null)`
var WriteBandwidthLastRecordQ = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_bw_write$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "bw", timestamp FROM "%s"."%s"."air" order by time desc limit 1`

var ReadIOPSAggRPQ = `SELECT /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_read$/, /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops" FROM "%s"."%s"."mean_air" WHERE time > now() - %s and time > %s FILL(null)`
var ReadIOPSDefaultRPQ = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_read$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "iops", median(unixTimestamp) as timestamp FROM "%s"."%s"."air" WHERE time > now() - %s and time > %s GROUP BY time(%s) FILL(null)`
var ReadIOPSLastRecordQ = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_read$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops", timestamp FROM "%s"."%s"."air" order by time desc limit 1`

var WriteIOPSAggRPQ = `SELECT /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_write$/, /^mean_perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops" FROM "%s"."%s"."mean_air" WHERE time > now() - %s and time > %s FILL(null)`
var WriteIOPSDefaultRPQ = `SELECT mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_write$/), mean(/^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/) as "iops", median(unixTimestamp) as timestamp FROM "%s"."%s"."air" WHERE time > now() - %s and time > %s GROUP BY time(%s) FILL(null)`
var WriteIOPSLastRecordQ = `SELECT /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_iops_write$/, /^perf_data_0_tid_arr_[\S]+_aid_arr_[\S]+_aid$/ as "iops", timestamp FROM "%s"."%s"."air" order by time desc limit 1`

var LatencyAggRPQ = `SELECT /^mean_lat_data_[\S]+_aid_arr_[\S]+_sid_arr_0_mean$/, /^mean_lat_data_[\S]+_aid_arr_[\S]+_aid$/ as "latency" FROM "%s"."%s"."mean_air" WHERE time > now() - %s and time > %s FILL(null)`
var LatencyDefaultRPQ = `SELECT mean(/^lat_data_[\S]+_aid_arr_[\S]+_sid_arr_0_mean$/), mean(/^lat_data_[\S]+_aid_arr_[\S]+_aid$/) as "latency", median(unixTimestamp) as timestamp FROM "%s"."%s"."air" WHERE time > now() - %s and time > %s GROUP BY time(%s) FILL(null)`
var LatencyLastRecordQ = `SELECT /^lat_data_[\S]+_aid_arr_[\S]+_sid_arr_0_mean$/, /^lat_data_[\S]+_aid_arr_[\S]+_aid$/ as "latency", timestamp FROM "%s"."%s"."air" order by time desc limit 1`

var RebuildingLogQ = `SELECT "value" FROM "%s"."autogen"."rebuilding_status" WHERE time > now() - %s`
