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
	gauge_metric_cnt              = 5
	gauge_metric_name_prefix      = "TestGaugeMetric_"
	gauge_metric_label_key_prefix = "TestGaugeLabel_"

	gauge_metric_value_range         = 0xFFFF
	gauge_metric_update_repeat_range = 10
)

func TestGauge(t *testing.T) {

	// Prepare exporter
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	// Prepare gauge metrics to be tested
	rand.Seed(time.Now().UnixNano())

	gauge_metric := make([]GaugeMetric, gauge_metric_cnt)

	for i := 0; i < gauge_metric_cnt; i++ {
		gauge_metric[i].name = gauge_metric_name_prefix + strconv.Itoa(i)
		gauge_metric[i].labels = &map[string]string{
			gauge_metric_label_key_prefix + "1": strconv.Itoa(i),
			gauge_metric_label_key_prefix + "2": strconv.Itoa(i) + strconv.Itoa(i),
		}

		gauge_metric[i].value = rand.Int63() % gauge_metric_value_range

		addGauge(&(gauge_metric[i]))

		t.Logf("Add: %s{%s} %d", gauge_metric[i].name, MakeLabelExporterFormat(gauge_metric[i].labels), gauge_metric[i].value)
	}

	// Add randomly gauge data
	rand.Seed(time.Now().UnixNano())

	for i := 0; i < gauge_metric_cnt; i++ {

		gauge_metric_update_repeat := rand.Int()%gauge_metric_update_repeat_range + 1

		for j := 0; j < gauge_metric_update_repeat; j++ {
			gauge_metric[i].value = rand.Int63() % gauge_metric_value_range
			addGauge(&(gauge_metric[i]))
			t.Logf("A gauge metric(%s) is updated", gauge_metric[i].name)
		}
	}

	// Get the result from exporter
	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)
	exported_data := string(mBody)

	// Check whether the gauge metric is correct
	for i := 0; i < gauge_metric_cnt; i++ {

		expected_data := fmt.Sprintf("%s{%s} %d", gauge_metric[i].name, MakeLabelExporterFormat(gauge_metric[i].labels), gauge_metric[i].value)
		isCorrect := strings.Contains(exported_data, expected_data)

		if isCorrect {
			t.Logf("A gauge metric(%s{%s} %d) is exported normally", gauge_metric[i].name, MakeLabelExporterFormat(gauge_metric[i].labels), gauge_metric[i].value)
		} else {
			t.Errorf("A gauge metric(%s{%s} %d) is exported abnormally", gauge_metric[i].name, MakeLabelExporterFormat(gauge_metric[i].labels), gauge_metric[i].value)
			t.Fail()
		}
	}
}
