package cmd

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"strings"
	"testing"
	"time"

	pb "github.com/poseidonos/pos-exporter/api/pos.metric"
	"google.golang.org/grpc"
)

const (
	TEST_RPC_HOST  = "localhost"
	TEST_RPC_PORT  = "50051"
	TEST_HTTP_HOST = "localhost"
	TEST_HTTP_PORT = "2112"
)

func TestOnline1(t *testing.T) {
	conn, err := grpc.Dial(fmt.Sprintf("%s:%s", TEST_RPC_HOST, TEST_RPC_PORT), grpc.WithInsecure(), grpc.WithBlock())

	var histogramValue pb.HistogramValue
	histogramValue.ZeroIndex = 0
	histogramValue.UpperBound = 100
	histogramValue.LowerBound = 0
	histogramValue.BucketScale = 10
	histogramValue.ScaleType = 0
	histogramValue.UnderflowCount = 0
	histogramValue.OverflowCount = 0
	histogramValue.BucketCount = []uint64{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

	var metric pb.Metric
	var label1 = pb.Label{Key: "Prop1", Value: "Val1"}
	var label2 = pb.Label{Key: "Prop2", Value: "Val2"}
	metric.Name = "HistogramTest"
	metric.Type = pb.MetricTypes_HISTOGRAM
	metric.Value = &pb.Metric_HistogramValue{HistogramValue: &histogramValue}
	metric.Labels = []*pb.Label{&label1, &label2}

	var addRequest pb.MetricPublishRequest = pb.MetricPublishRequest{Metrics: []*pb.Metric{&metric}}
	client := pb.NewMetricManagerClient(conn)
	res, err := client.MetricPublish(context.Background(), &addRequest)
	fmt.Println(res, err)

	time.Sleep(time.Second)
	httpres, err := http.Get(fmt.Sprintf("http://%s:%s/metrics", TEST_HTTP_HOST, TEST_HTTP_PORT))
	bodyString, err := io.ReadAll(httpres.Body)
	strMetric := string(bodyString)
	for _, line := range strings.Split(strMetric, "\n") {
		if strings.Contains(line, "HistogramTest") {
			fmt.Println(line)
		}
	}

}

func TestOnline2_DifferentValues(t *testing.T) {
	conn, _ := grpc.Dial(fmt.Sprintf("%s:%s", TEST_RPC_HOST, TEST_RPC_PORT), grpc.WithInsecure(), grpc.WithBlock())

	for valId := 0; valId < 10; valId++ {
		var histogramValue pb.HistogramValue
		histogramValue.ZeroIndex = 0
		histogramValue.UpperBound = 50
		histogramValue.LowerBound = 0
		histogramValue.BucketScale = 10
		histogramValue.ScaleType = 0
		histogramValue.UnderflowCount = 0
		histogramValue.OverflowCount = 0
		histogramValue.BucketCount = []uint64{1, 2, 3, 4, 5}

		var metric pb.Metric
		var label1 = pb.Label{Key: "Prop1", Value: fmt.Sprintf("mval_%d", valId)}
		var label2 = pb.Label{Key: "Prop2", Value: fmt.Sprintf("mval2_%d", valId)}
		metric.Name = "HistogramTest"
		metric.Type = pb.MetricTypes_HISTOGRAM
		metric.Value = &pb.Metric_HistogramValue{HistogramValue: &histogramValue}
		metric.Labels = []*pb.Label{&label1, &label2}

		var addRequest pb.MetricPublishRequest = pb.MetricPublishRequest{Metrics: []*pb.Metric{&metric}}
		client := pb.NewMetricManagerClient(conn)
		res, err := client.MetricPublish(context.Background(), &addRequest)
		fmt.Println(res, err)
	}

	time.Sleep(time.Second)
	httpres, _ := http.Get(fmt.Sprintf("http://%s:%s/metrics", TEST_HTTP_HOST, TEST_HTTP_PORT))
	bodyString, _ := io.ReadAll(httpres.Body)
	strMetric := string(bodyString)
	for _, line := range strings.Split(strMetric, "\n") {
		if strings.Contains(line, "HistogramTest") {
			fmt.Println(line)
		}
	}

}
