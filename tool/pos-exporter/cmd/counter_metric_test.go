package cmd

import (
	"fmt"
	"math/rand"
	"strconv"
	"strings"
	"testing"
	"time"

	"io/ioutil"
	"net/http"
	"net/http/httptest"

	"github.com/prometheus/client_golang/prometheus/promhttp"
)

const (
	counter_metric_cnt              = 5
	counter_metric_name_prefix      = "TestCounterMetric_"
	counter_metric_label_key_prefix = "TestCounterLabel_"

	counter_metric_repeat_range = 10
	counter_metric_stride_range = 0xFFFF
)

func TestCounter(t *testing.T) {

	// Prepare exporter
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	// Prepare counter metrics to be tested
	counterMetric := make([]CounterMetric, counter_metric_cnt)

	for i := 0; i < counter_metric_cnt; i++ {
		counterMetric[i].name = counter_metric_name_prefix + strconv.Itoa(i)
		counterMetric[i].labels = &map[string]string{
			counter_metric_label_key_prefix + "1": strconv.Itoa(i),
			counter_metric_label_key_prefix + "2": strconv.Itoa(i) + strconv.Itoa(i),
		}

		counterMetric[i].value = uint64(i * i)
		addCounter(&(counterMetric[i]))
		t.Logf("Add: %s{%s} %d", counterMetric[i].name, MakeLabelExporterFormat(counterMetric[i].labels), counterMetric[i].value)
	}

	// Add randomly counter data
	counterMetricRepeat := make([]int, counter_metric_cnt)
	counterMetricStride := make([]int, counter_metric_cnt)

	rand.Seed(time.Now().UnixNano())

	for i := 0; i < counter_metric_cnt; i++ {
		counterMetricRepeat[i] = rand.Intn(counter_metric_repeat_range)
		counterMetricStride[i] = rand.Intn(counter_metric_stride_range)

		counterMetric[i].value = uint64(counterMetricStride[i])

		for j := 0; j < counterMetricRepeat[i]; j++ {
			addCounter(&(counterMetric[i]))
		}
	}

	// Get the result from exporter
	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)
	exported_data := string(mBody)

	// Check whether the counter metric  is correct
	for i := 0; i < counter_metric_cnt; i++ {

		expected_data := fmt.Sprintf("%s{%s} %d", counterMetric[i].name, MakeLabelExporterFormat(counterMetric[i].labels), uint64(i*i+counterMetricRepeat[i]*counterMetricStride[i]))
		isCorrect := strings.Contains(exported_data, expected_data)

		if isCorrect {
			t.Logf("A counter metric(%s{%s} %d) is exported normally", counterMetric[i].name, MakeLabelExporterFormat(counterMetric[i].labels), uint64(i*i+counterMetricRepeat[i]*counterMetricStride[i]))
		} else {
			t.Errorf("A counter metric(%s{%s} %d) is exported abnormally", counterMetric[i].name, MakeLabelExporterFormat(counterMetric[i].labels), uint64(i*i+counterMetricRepeat[i]*counterMetricStride[i]))
			t.Fail()
		}
	}
}
