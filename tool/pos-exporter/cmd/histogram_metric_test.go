package cmd

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"strconv"
	"strings"
	"testing"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

func equalStringArray(original []string, exp []string) bool {
	if len(original) != len(exp) {
		return false
	}

	for index := range exp {
		if exp[index] != original[index] {
			return false
		}
	}
	return true

}

func Test_GetSortedKey(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1", "prop2": "prop_v2"}
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}
	hMetric.bucketRange = []int64{0, 10, 20, 30, 40}
	hMetric.sum = 100
	hMetric.totalCount = 4

	var sortedKey = getSortedKeyFromLabelMap(hMetric.labels)
	fmt.Println(sortedKey)
	if !equalStringArray(sortedKey, []string{"prop1", "prop2"}) {
		t.Fail()
	}

	hMetric.labels = &map[string]string{"prop3": "prop_v1", "prop1": "prop_v2"}
	sortedKey = getSortedKeyFromLabelMap(hMetric.labels)
	fmt.Println(sortedKey)
	if equalStringArray(sortedKey, []string{"prop1", "prop2"}) {
		t.Fail()
	}
	if !equalStringArray(sortedKey, []string{"prop1", "prop3"}) {
		t.Fail()
	}

	hMetric.labels = &map[string]string{"aaaa": "prop_v1", "aaa": "prop_v2", "a": "prop_v2", "3": "prop_v2"}
	sortedKey = getSortedKeyFromLabelMap(hMetric.labels)
	fmt.Println(sortedKey)
	if equalStringArray(sortedKey, []string{"aaa", "aa", "a", "3"}) {
		t.Fail()
	}
	if !equalStringArray(sortedKey, []string{"3", "a", "aaa", "aaaa"}) {
		t.Fail()
	}

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

func Test_getMapIndexString(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1", "prop2": "prop_v2"}
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}
	hMetric.bucketRange = []int64{0, 10, 20, 30, 40}
	hMetric.sum = 100
	hMetric.totalCount = 4

	var idxString = getMapIndexString(hMetric.labels)
	fmt.Println(idxString)
	if idxString != "prop1-prop_v1,prop2-prop_v2" {
		t.Fail()
	}

	hMetric.labels = &map[string]string{"prop3": "prop_v1", "prop1": "prop_v2"}
	idxString = getMapIndexString(hMetric.labels)
	fmt.Println(idxString)
	if idxString != "prop1-prop_v2,prop3-prop_v1" {
		t.Fail()
	}

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

/* Test store.go */
func Test_histogramFunc1(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "internal_hist_test1_func1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}
	hMetric.bucketRange = []int64{0, 10, 20, 30, 40}
	hMetric.sum = 100
	hMetric.totalCount = 4

	addHistogram(&hMetric)

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

func Test_POSHistogramCollector(t *testing.T) {

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1_test1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}
	hMetric.bucketRange = []int64{0, 10, 20, 30, 40}
	hMetric.sum = 100
	hMetric.totalCount = 4

	_, exists := histogramMap[hMetric.name]
	if exists {
		fmt.Println("Already Exists")
		t.Fail()
	}

	var histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
	histogramMap[hMetric.name] = histVector
	prometheus.MustRegister(histVector)

	// add histogram
	hist := histVector.FindHistogram(hMetric.labels)
	if hist != nil {
		t.Fail()
	}
	hist = NewPOSHistogram(len(hMetric.bucketCount))
	histVector.AddHistogram(hMetric.labels, hist)

	// re find
	hist = histVector.FindHistogram(hMetric.labels)
	if hist == nil {
		t.Fail()
	}
	t.Logf("Found Histogram : %v", hist)

	// Update Label Key, Value
	var labelKeys = getSortedKeyFromLabelMap(hMetric.labels)
	var labelValues = make([]string, len(labelKeys))
	for idx := range labelKeys {
		labelValues[idx] = (*hMetric.labels)[labelKeys[idx]]
	}
	hist.UpdateLabelKey(labelKeys)
	hist.UpdateLabelValues(labelValues)
	hist.UpdateHistogram(&hMetric)

	// prometheus http test
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)

	var exp []string = make([]string, 10)
	exp[0] = "# HELP histogram1_test1 POS Histogram"
	exp[1] = "# TYPE histogram1_test1 histogram"
	exp[2] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"0\"} 0"
	exp[3] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"10\"} 1"
	exp[4] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"20\"} 2"
	exp[5] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"30\"} 3"
	exp[6] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"40\"} 4"
	exp[7] = "histogram1_test1_bucket{prop1=\"prop_v1\",le=\"+Inf\"} 4"
	exp[8] = "histogram1_test1_sum{prop1=\"prop_v1\"} 100"
	exp[9] = "histogram1_test1_count{prop1=\"prop_v1\"} 4"

	var original = string(mBody)
	//fmt.Println(original)
	for expIdx := 0; expIdx < 10; expIdx++ {
		if !strings.Contains(original, exp[expIdx]) {
			fmt.Printf("[%s] does not in prometheus metrics text", exp[expIdx])
			t.Fail()
		}
	}
	prometheus.Unregister(histVector)

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

func Test_POSHistogramCollector_DifferentValues(t *testing.T) {

	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram2"
	hMetric.labels = &map[string]string{"prop1": "prop_v1", "prop2": "prop_v2"}
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}
	hMetric.bucketRange = []int64{2, 4, 8, 16, 32}

	var hMetric1 = HistogramMetric{}
	hMetric1.name = "histogram2"
	hMetric1.labels = &map[string]string{"prop1": "prop_v111", "prop2": "prop_vqqq"}
	hMetric1.bucketCount = []uint64{1, 2, 3, 4, 5}
	hMetric1.bucketRange = []int64{2, 4, 8, 16, 32}

	/* register histogram vector */
	var mapIndex = strings.Join(getSortedKeyFromLabelMap(hMetric.labels), ",")
	_, exists := histogramMap[mapIndex]
	if exists {
		t.Fail()
	}
	fmt.Println("Test_POSHistogramCollector_cleDifferentValues, MapIndex :: ", mapIndex)

	var histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
	histogramMap[mapIndex] = histVector
	prometheus.MustRegister(histVector)

	if histVector.FindHistogram(hMetric.labels) != nil {
		t.Fail()
	}
	if histVector.FindHistogram(hMetric1.labels) != nil {
		t.Fail()
	}

	/* Test Register */
	hist := NewPOSHistogram(len(hMetric.bucketCount))
	hist1 := NewPOSHistogram(len(hMetric1.bucketCount))
	histVector.AddHistogram(hMetric.labels, hist)
	histVector.AddHistogram(hMetric1.labels, hist1)

	// Update Label Key, Value
	var labelKeys = getSortedKeyFromLabelMap(hMetric.labels)
	var labelValues = make([]string, len(labelKeys))
	for idx := range labelKeys {
		labelValues[idx] = (*hMetric.labels)[labelKeys[idx]]
	}
	hist.UpdateLabelKey(labelKeys)
	hist.UpdateLabelValues(labelValues)
	hist.UpdateHistogram(&hMetric)

	labelKeys = getSortedKeyFromLabelMap(hMetric1.labels)
	labelValues = make([]string, len(labelKeys))
	for idx := range labelKeys {
		labelValues[idx] = (*hMetric1.labels)[labelKeys[idx]]
	}
	hist1.UpdateLabelKey(labelKeys)
	hist1.UpdateLabelValues(labelValues)
	hist1.UpdateHistogram(&hMetric)

	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)
	var original = string(mBody)

	if !strings.Contains(original, "prop_v2") {
		t.Fail()
	}

	if !strings.Contains(original, "prop_vqqq") {
		t.Fail()
	}

	prometheus.Unregister(histVector)

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

func Test_POSHistogramCollector_BucketChangeTest(t *testing.T) {

	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram3"
	hMetric.labels = &map[string]string{"prop3": "prop_v3333", "prop4": "prop_v4444"}
	hMetric.bucketRange = []int64{100, 1000, 10000, 100000, 1000000}
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	/* register histogram vector */
	var mapIndex = hMetric.name
	_, exists := histogramMap[mapIndex]
	if exists {
		t.Errorf("histogram must not exists")
		t.Fail()
	}

	var histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
	histogramMap[mapIndex] = histVector
	prometheus.MustRegister(histVector)

	if histVector.FindHistogram(hMetric.labels) != nil {
		t.Errorf("FindHistogram not nil")
		t.Fail()
	}

	/* Test Register */
	hist := NewPOSHistogram(len(hMetric.bucketCount))
	histVector.AddHistogram(hMetric.labels, hist)

	// Update Label Key, Value
	for testLoop := 0; testLoop < 10; testLoop++ {
		var labelKeys = getSortedKeyFromLabelMap(hMetric.labels)
		var labelValues = make([]string, len(labelKeys))
		for idx := range labelKeys {
			labelValues[idx] = (*hMetric.labels)[labelKeys[idx]]
		}
		hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}
		for i := 0; i < 5; i++ {
			hMetric.bucketCount[i] += uint64(testLoop)
		}

		hist.UpdateLabelKey(labelKeys)
		hist.UpdateLabelValues(labelValues)
		hist.UpdateHistogram(&hMetric)

		res, _ := http.Get(ts.URL)
		mBody, _ := ioutil.ReadAll(res.Body)
		var original = string(mBody)
		var expSum uint64 = 0

		for _, v := range hMetric.bucketCount {
			expSum += v
		}

		for _, line := range strings.Split(original, "\n") {
			if strings.Contains(line, "histogram2_count") {
				sp := strings.Split(line, " ")
				actualSum, _ := strconv.Atoi(sp[1])
				if uint64(actualSum) != expSum {
					t.Errorf("exp,actual = %d %d", expSum, actualSum)
					t.Fail()
				}
			}
		}
	}

	prometheus.Unregister(histVector)

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}

func Test_POSHistogramAddRemove(t *testing.T) {

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram5"
	hMetric.labels = &map[string]string{"s_key_1": "s_value_1"}
	hMetric.bucketRange = []int64{0, 4, 8, 16, 128}
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	var mapIndex = strings.Join(getSortedKeyFromLabelMap(hMetric.labels), ",")
	histVector := NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
	histogramMap[mapIndex] = histVector
	prometheus.MustRegister(histVector)

	// add histogram
	hist := histVector.FindHistogram(hMetric.labels)
	if hist != nil {
		t.Fail()
	}
	hist = NewPOSHistogram(len(hMetric.bucketCount))
	histVector.AddHistogram(hMetric.labels, hist)

	// re find
	hist = histVector.FindHistogram(hMetric.labels)
	if hist == nil {
		t.Fail()
	}

	// Update Label Key, Value
	var labelKeys = getSortedKeyFromLabelMap(hMetric.labels)
	var labelValues = make([]string, len(labelKeys))
	for idx := range labelKeys {
		labelValues[idx] = (*hMetric.labels)[labelKeys[idx]]
	}
	hist.UpdateLabelKey(labelKeys)
	hist.UpdateLabelValues(labelValues)
	hist.UpdateHistogram(&hMetric)

	// prometheus http test
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)

	var original = string(mBody)
	if !strings.Contains(original, "histogram5") {
		fmt.Println("histogram5 doesn't exists ")
		t.Fail()
	} else {
		if !strings.Contains(original, "s_key_1") || !strings.Contains(original, "s_value_1") {
			fmt.Println("histogram5 doesn't exists ")
			t.Fail()
		}
	}

	// remove Test
	var rMetric = HistogramMetric{}
	rMetric.name = "histogram5"
	rMetric.labels = &map[string]string{"s_key_1": "s_value_NONONONO"}
	rMetric.bucketRange = []int64{0, 4, 8, 16, 128}
	rMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	if histVector.FindHistogram(rMetric.labels) != nil {
		fmt.Println("Expect nil but not")
		t.Fail()
	}

	var removedCount = histVector.RemoveHistogram(rMetric.labels)
	if removedCount != false {
		fmt.Println("Expect removed count = 0, but not ", removedCount)
		t.Fail()
	}

	// remove actual metric
	rMetric.labels = &map[string]string{"s_key_1": "s_value_1"}
	if histVector.FindHistogram(rMetric.labels) == nil {
		fmt.Println("Expect not nil, but return nil")
		t.Fail()
	}

	removedCount = histVector.RemoveHistogram(rMetric.labels)
	if removedCount == false {
		fmt.Println("Expect removed count = 1, but not ", removedCount)
		t.Fail()
	}

	if histVector.FindHistogram(rMetric.labels) != nil {
		fmt.Println("Expect nil after removing actual metric")
		t.Fail()
	}

	res, _ = http.Get(ts.URL)
	mBody, _ = ioutil.ReadAll(res.Body)
	original = string(mBody)
	if strings.Contains(original, "histogram5") {
		fmt.Println("histogram5 exists after removing")
		t.Fail()
	} else {
		if strings.Contains(original, "s_key_1") || strings.Contains(original, "s_value_1") {
			fmt.Println("histogram5's property exists after removing")
			t.Fail()
		}
	}

	prometheus.Unregister(histVector)

	if !t.Failed() {
		t.Logf("Success Test [%s]", t.Name())
	} else {
		t.Errorf("Failed Test [%s]", t.Name())
	}
}
