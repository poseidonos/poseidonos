package cmd

import (
	"context"
	"fmt"
	"io/ioutil"
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
	conn, _ := grpc.Dial(fmt.Sprintf("%s:%s", TEST_RPC_HOST, TEST_RPC_PORT), grpc.WithInsecure(), grpc.WithBlock())

	var histogramValue pb.HistogramValue
	histogramValue.BucketCount = []uint64{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
	histogramValue.BucketRange = []int64{1, 2, 4, 8, 16, 32, 64, 128, 256, 512}
	histogramValue.TotalCount = 10
	histogramValue.Sum = 0

	for i := 0; i < 10; i++ {
		histogramValue.Sum += histogramValue.BucketRange[i]
	}

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
	t.Logf("Total Received : %d", res.TotalReceivedMetrics)
	if err != nil {
		t.Fail()
	}

	time.Sleep(time.Second)
	httpres, _ := http.Get(fmt.Sprintf("http://%s:%s/metrics", TEST_HTTP_HOST, TEST_HTTP_PORT))
	bodyString, _ := ioutil.ReadAll(httpres.Body)
	strMetric := string(bodyString)
	for _, line := range strings.Split(strMetric, "\n") {
		if strings.Contains(line, "HistogramTest") {
			fmt.Println(line)
		}
	}

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Logf("Failed Test [%s]", t.Name())
	}

}

func TestOnline2_DifferentValues(t *testing.T) {
	conn, _ := grpc.Dial(fmt.Sprintf("%s:%s", TEST_RPC_HOST, TEST_RPC_PORT), grpc.WithInsecure(), grpc.WithBlock())

	for valId := 0; valId < 10; valId++ {
		var histogramValue pb.HistogramValue
		histogramValue.BucketCount = []uint64{1, 2, 3, 4, 5}
		histogramValue.BucketRange = []int64{-10, 0, 10, 20, 30}
		histogramValue.Sum = 50
		histogramValue.TotalCount = 5

		var metric pb.Metric
		var label1 = pb.Label{Key: "Prop1", Value: fmt.Sprintf("mval_%d", valId)}
		var label2 = pb.Label{Key: "Prop2", Value: fmt.Sprintf("mval2_%d", valId)}
		metric.Name = "HistogramTestDiffValue"
		metric.Type = pb.MetricTypes_HISTOGRAM
		metric.Value = &pb.Metric_HistogramValue{HistogramValue: &histogramValue}
		metric.Labels = []*pb.Label{&label1, &label2}

		var addRequest pb.MetricPublishRequest = pb.MetricPublishRequest{Metrics: []*pb.Metric{&metric}}
		client := pb.NewMetricManagerClient(conn)
		res, err := client.MetricPublish(context.Background(), &addRequest)
		t.Logf("Total Received : %d", res.TotalReceivedMetrics)
		if err != nil {
			t.FailNow()
		}
	}

	time.Sleep(time.Second)
	httpres, _ := http.Get(fmt.Sprintf("http://%s:%s/metrics", TEST_HTTP_HOST, TEST_HTTP_PORT))
	bodyString, _ := ioutil.ReadAll(httpres.Body)
	strMetric := string(bodyString)
	for _, line := range strings.Split(strMetric, "\n") {
		if strings.Contains(line, "HistogramTestDiffValue") {
			fmt.Println(line)
		}
	}

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Logf("Failed Test [%s]", t.Name())
	}

}
