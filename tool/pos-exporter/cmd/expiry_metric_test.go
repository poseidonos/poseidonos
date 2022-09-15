package cmd

import (
	"io/ioutil"
	"math/rand"
	"net/http"
	"net/http/httptest"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/prometheus/client_golang/prometheus/promhttp"
)

const (
	expiry_counter_metric_cnt      = 5
	expiry_gauge_metric_cnt        = 5
	expiry_poshistogram_metric_cnt = 5

	metric_update_cnt          = 3
	metric_update_range_second = 5

	expiry_metric_name_prefix = "TestMetricExpiry_"
	expiry_label_key_prefix   = "TestMetricExpiryLabel_"
)

func TestExpiry(t *testing.T) {

	// Prepare exporter
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	// Prepare metrics to be tested
	expiry_total_metric_cnt := expiry_counter_metric_cnt + expiry_gauge_metric_cnt + expiry_poshistogram_metric_cnt
	update_time := make([]int64, expiry_total_metric_cnt)

	// 1) Add counter metric
	counterMetric := make([]CounterMetric, expiry_counter_metric_cnt)

	for i := 0; i < expiry_counter_metric_cnt; i++ {
		counterMetric[i].name = expiry_metric_name_prefix + strconv.Itoa(i)
		counterMetric[i].labels = &map[string]string{"k": "v", "publisher_name": "air_delegator"}
		counterMetric[i].value = 1

		addCounter(&(counterMetric[i]))
		update_time[i] = time.Now().Unix()
		t.Logf("Add: %s", counterMetric[i].name)
	}

	// 2) Add gauge metric
	gaugeMetric := make([]GaugeMetric, expiry_gauge_metric_cnt)

	for i := 0; i < expiry_gauge_metric_cnt; i++ {
		gaugeMetric[i].name = expiry_metric_name_prefix + strconv.Itoa(i+expiry_counter_metric_cnt)
		gaugeMetric[i].labels = &map[string]string{"k": "v", "publisher_name": "air_delegator"}
		gaugeMetric[i].value = 1

		addGauge(&(gaugeMetric[i]))
		update_time[i+expiry_counter_metric_cnt] = time.Now().Unix()
		t.Logf("Add: %s", gaugeMetric[i].name)
	}

	// Add poshistogram metric
	histogramMetric := make([]HistogramMetric, expiry_poshistogram_metric_cnt)
	for i := 0; i < expiry_poshistogram_metric_cnt; i++ {
		histogramMetric[i].name = expiry_metric_name_prefix + strconv.Itoa(i+expiry_counter_metric_cnt+expiry_gauge_metric_cnt)
		histogramMetric[i].labels = &map[string]string{"k": "v", "publisher_name": "air_delegator"}
		histogramMetric[i].bucketRange = []int64{0, 10, 20, 30}
		histogramMetric[i].bucketCount = []uint64{0, 0, 0, 0}
		histogramMetric[i].sum = 0
		histogramMetric[i].totalCount = 0

		addHistogram(&(histogramMetric[i]))
		update_time[i+expiry_counter_metric_cnt+expiry_gauge_metric_cnt] = time.Now().Unix()
		t.Logf("Add: %s", histogramMetric[i].name)
	}

	// Update metric periodically(random)
	t.Logf("Expiry validation start!")
	t.Logf("This test will sleep(for randomly) %d times...", expiry_total_metric_cnt*metric_update_cnt)

	rand.Seed(time.Now().UnixNano())

	for i := 0; i < expiry_total_metric_cnt*metric_update_cnt; i++ {

		// Randomly choose sleep time
		sleepTime := rand.Int()%metric_update_range_second + 1
		t.Logf("Sleep(%02d) for %d seconds...", i+1, sleepTime)
		time.Sleep((time.Duration(sleepTime)) * time.Second)

		// Randomly choose metric to be updated
		index := rand.Int() % expiry_total_metric_cnt

		if index < expiry_counter_metric_cnt {
			ii := index
			addCounter(&(counterMetric[ii]))
			t.Logf("%s metric is updated", counterMetric[ii].name)
		} else if index < expiry_counter_metric_cnt+expiry_gauge_metric_cnt {
			ii := index - expiry_counter_metric_cnt
			addGauge(&(gaugeMetric[ii]))
			t.Logf("%s metric is updated", gaugeMetric[ii].name)
		} else {
			ii := index - expiry_counter_metric_cnt - expiry_gauge_metric_cnt
			addHistogram(&(histogramMetric[ii]))
			t.Logf("%s metric is updated", histogramMetric[ii].name)
		}

		update_time[index] = time.Now().Unix()
	}

	// Run expiry Manager
	go runExpiryManager()
	time.Sleep(2 * time.Second)

	// Get the result from exporter
	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)
	exported_data := string(mBody)

	// Check if a metric remains or is expired normally
	now := time.Now().Unix()

	for i := 0; i < expiry_total_metric_cnt; i++ {

		isCorrect := true
		contain := true
		ii := expiry_total_metric_cnt
		name := ""

		if i < expiry_counter_metric_cnt {
			ii = i
			contain = strings.Contains(exported_data, counterMetric[ii].name)
			name = counterMetric[ii].name
		} else if i < expiry_counter_metric_cnt+expiry_gauge_metric_cnt {
			ii = i - expiry_counter_metric_cnt
			contain = strings.Contains(exported_data, gaugeMetric[ii].name)
			name = gaugeMetric[ii].name
		} else {
			ii = i - expiry_counter_metric_cnt - expiry_gauge_metric_cnt
			contain = strings.Contains(exported_data, histogramMetric[ii].name)
			name = histogramMetric[ii].name
		}

		if now-update_time[i] > valid_duration_second && contain {
			isCorrect = false
		} else if now-update_time[i] <= valid_duration_second && !contain {
			isCorrect = false
		}

		if isCorrect {
			if contain {
				t.Logf("A metric(%s) is exported normally", name)
			} else {
				t.Logf("A metric(%s) is expired normally", name)
			}
		} else {
			if contain {
				t.Errorf("A metric(%s) is exported but it should be expired", name)
			} else {
				t.Errorf("A metric(%s) is expired but it should be exported", name)
			}
			t.Fail()
		}
	}
}
