package cmd

import (
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"strconv"
	"strings"
	"testing"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

func equalUpperBound(original []float64, exp []int) bool {
	if len(original) != len(exp) {
		return false
	}
	for index := range exp {
		if exp[index] != int(original[index]) {
			return false
		}
	}
	return true
}

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
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0

	hMetric.lowerBound = 0
	hMetric.upperBound = 50
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 10
	hMetric.scaleType = 0 /* linear */
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}

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

}

func Test_getMapIndexString(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1", "prop2": "prop_v2"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0

	hMetric.lowerBound = 0
	hMetric.upperBound = 50
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 10
	hMetric.scaleType = 0 /* linear */
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}

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
}

/* Test store.go */
func Test_histogramFunc1(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0

	hMetric.lowerBound = 0
	hMetric.upperBound = 50
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 10
	hMetric.scaleType = 0 /* linear */
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}

}

func Test_UpperBoundLinear(t *testing.T) {
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0

	hMetric.lowerBound = 0
	hMetric.upperBound = 50
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 10
	hMetric.scaleType = 0 /* linear */
	hMetric.bucketCount = []uint64{0, 1, 2, 3, 4}

	var upper = CalculateUpperBound(&hMetric)
	var exp = []int{10, 20, 30, 40, 50}
	fmt.Println(upper, exp)
	if !equalUpperBound(upper, exp) {
		t.Fail()
	}

	hMetric.lowerBound = -50
	hMetric.upperBound = 50
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 10
	hMetric.bucketCount = make([]uint64, 10)
	upper = CalculateUpperBound(&hMetric)
	exp = []int{-40, -30, -20, -10, 0, 10, 20, 30, 40, 50}
	fmt.Println(upper, exp)
	if !equalUpperBound(upper, exp) {
		t.Fail()
	}

	hMetric.lowerBound = 0
	hMetric.upperBound = 512 * 16
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 512
	hMetric.bucketCount = make([]uint64, 16)
	upper = CalculateUpperBound(&hMetric)
	exp = []int{512, 1024, 1536, 2048, 2560, 3072, 3584, 4096, 4608, 5120, 5632, 6144, 6656, 7168, 7680, 8192}
	fmt.Println(upper, exp)
	if !equalUpperBound(upper, exp) {
		t.Fail()
	}
}

func Test_UpperBoundExpon(t *testing.T) {
	//t.Logf("Test UpperBoundExpon")
	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0

	/*
		LowerBound -100, UpperBound +100, Scale 10 (linear type)
		Bucket Range   [-10^2,-10^1) [-10^1,-10^0) [-10^0,0) [0,10^0) [10^0,10^1) [10^1,10^2]
		Bucket Index    0                 1            2        3         4           5
		Note                                                  ZeroIdx
		UpperBound      -10^1         -10^0         0         1        10^1        10^2
	*/
	hMetric.lowerBound = -100
	hMetric.upperBound = 100
	hMetric.zeroIndex = 3
	hMetric.bucketScale = 10
	hMetric.scaleType = 1 /* exp */
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	var upper = CalculateUpperBound(&hMetric)
	var exp = []int{-10, -1, 0, 1, 10}
	fmt.Println(upper, exp)
	if !equalUpperBound(upper, exp) {
		t.Fail()
	}

	hMetric.lowerBound = 0
	hMetric.upperBound = 8192
	hMetric.zeroIndex = 0
	hMetric.bucketScale = 2
	hMetric.scaleType = 1 /* exp */
	hMetric.bucketCount = make([]uint64, 14)

	upper = CalculateUpperBound(&hMetric)
	exp = []int{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192}
	fmt.Println(upper, exp)
	if !equalUpperBound(upper, exp) {
		t.Fail()
	}

}

func Test_POSHistogramCollector(t *testing.T) {

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram1"
	hMetric.labels = &map[string]string{"prop1": "prop_v1"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0
	hMetric.lowerBound = -100
	hMetric.upperBound = 100
	hMetric.zeroIndex = 3
	hMetric.bucketScale = 10
	hMetric.scaleType = 1 /* exp */
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	var mapIndex = strings.Join(getSortedKeyFromLabelMap(hMetric.labels), ",")
	histVector, exists := histogramMap[mapIndex]
	if exists {
		fmt.Println("Already Exists")
		t.Fail()
	}
	fmt.Println("Test_POSHistogramCollector, MapIndex :: ", mapIndex)

	histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
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
	fmt.Println(hist)

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
	fmt.Println("URL : ", ts.URL, ts)
	defer ts.Close()

	res, err := http.Get(ts.URL)
	mBody, err := io.ReadAll(res.Body)
	fmt.Println(err)

	var exp []string = make([]string, 1024)
	exp[0] = "# HELP histogram1 POS Histogram"
	exp[1] = "# TYPE histogram1 histogram"
	exp[2] = "histogram1_bucket{prop1=\"prop_v1\",le=\"-10\"} 1"
	exp[3] = `histogram1_bucket{prop1="prop_v1",le="-1"} 3`
	exp[4] = `histogram1_bucket{prop1="prop_v1",le="0"} 6`
	exp[5] = `histogram1_bucket{prop1="prop_v1",le="1"} 10`
	exp[6] = `histogram1_bucket{prop1="prop_v1",le="10"} 15`
	exp[7] = `histogram1_bucket{prop1="prop_v1",le="+Inf"} 15`
	exp[8] = `histogram1_sum{prop1="prop_v1"} 0`
	exp[9] = `histogram1_count{prop1="prop_v1"} 15`

	var original = string(mBody)
	for expIdx := 0; expIdx < 10; expIdx++ {
		if !strings.Contains(original, exp[expIdx]) {
			t.Fail()
		}
	}
	prometheus.Unregister(histVector)

}

func Test_POSHistogramCollector_DifferentValues(t *testing.T) {

	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram2"
	hMetric.labels = &map[string]string{"prop1": "prop_v1", "prop2": "prop_v2"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0
	hMetric.lowerBound = -100
	hMetric.upperBound = 100
	hMetric.zeroIndex = 3
	hMetric.bucketScale = 10
	hMetric.scaleType = 1 /* exp */
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	var hMetric1 = HistogramMetric{}
	hMetric1.name = "histogram2"
	hMetric1.labels = &map[string]string{"prop1": "prop_v111", "prop2": "prop_vqqq"}
	hMetric1.underflowCount = 0
	hMetric1.overflowCount = 0
	hMetric1.lowerBound = -100
	hMetric1.upperBound = 100
	hMetric1.zeroIndex = 3
	hMetric1.bucketScale = 10
	hMetric1.scaleType = 1 /* exp */
	hMetric1.bucketCount = []uint64{1, 2, 3, 4, 5}

	/* register histogram vector */
	var mapIndex = strings.Join(getSortedKeyFromLabelMap(hMetric.labels), ",")
	histVector, exists := histogramMap[mapIndex]
	if exists {
		t.Fail()
	}
	fmt.Println("Test_POSHistogramCollector_cleDifferentValues, MapIndex :: ", mapIndex)

	histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
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
	mBody, _ := io.ReadAll(res.Body)
	var original = string(mBody)

	if !strings.Contains(original, "prop_v2") {
		t.Fail()
	}

	if !strings.Contains(original, "prop_vqqq") {
		t.Fail()
	}

	prometheus.Unregister(histVector)
}

func Test_POSHistogramCollector_BucketChangeTest(t *testing.T) {

	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram3"
	hMetric.labels = &map[string]string{"prop3": "prop_v3333", "prop4": "prop_v4444"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0
	hMetric.lowerBound = -100
	hMetric.upperBound = 100
	hMetric.zeroIndex = 3
	hMetric.bucketScale = 10
	hMetric.scaleType = 1 /* exp */
	hMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	/* register histogram vector */
	var mapIndex = strings.Join(getSortedKeyFromLabelMap(hMetric.labels), ",")
	histVector, exists := histogramMap[mapIndex]
	if exists {
		fmt.Println("already exists")
		t.Fail()
	}
	fmt.Println("Test_POSHistogramCollector_BucketChangeTest, MapIndex :: ", mapIndex)

	histVector = NewPOSHistogramCollector(hMetric.name, getSortedKeyFromLabelMap(hMetric.labels))
	histogramMap[mapIndex] = histVector
	prometheus.MustRegister(histVector)

	if histVector.FindHistogram(hMetric.labels) != nil {
		fmt.Println("FindHistogram not nil")
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
		hMetric.bucketCount = []uint64{1 + uint64(testLoop), 2, 3, 4, 5 + uint64(testLoop)}

		hist.UpdateLabelKey(labelKeys)
		hist.UpdateLabelValues(labelValues)
		hist.UpdateHistogram(&hMetric)

		res, _ := http.Get(ts.URL)
		mBody, _ := io.ReadAll(res.Body)
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
					fmt.Println("exp,actual = ", expSum, actualSum)
					t.Fail()
				}
			}
		}
	}

	prometheus.Unregister(histVector)

}

func Test_POSHistogramAddRemove(t *testing.T) {

	var hMetric = HistogramMetric{}
	hMetric.name = "histogram5"
	hMetric.labels = &map[string]string{"s_key_1": "s_value_1"}
	hMetric.underflowCount = 0
	hMetric.overflowCount = 0
	hMetric.lowerBound = -100
	hMetric.upperBound = 100
	hMetric.zeroIndex = 3
	hMetric.bucketScale = 10
	hMetric.scaleType = 1 /* exp */
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
	fmt.Println("URL : ", ts.URL, ts)
	defer ts.Close()

	res, _ := http.Get(ts.URL)
	mBody, _ := io.ReadAll(res.Body)

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
	rMetric.underflowCount = 0
	rMetric.overflowCount = 0
	rMetric.lowerBound = -100
	rMetric.upperBound = 100
	rMetric.zeroIndex = 3
	rMetric.bucketScale = 10
	rMetric.scaleType = 1 /* exp */
	rMetric.bucketCount = []uint64{1, 2, 3, 4, 5}

	if histVector.FindHistogram(rMetric.labels) != nil {
		fmt.Println("Expect nil but not")
		t.Fail()
	}

	var removedCount = histVector.RemoveHistogram(rMetric.labels)
	if removedCount != 0 {
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
	if removedCount == 0 {
		fmt.Println("Expect removed count = 1, but not ", removedCount)
		t.Fail()
	}

	if histVector.FindHistogram(rMetric.labels) != nil {
		fmt.Println("Expect nil after removing actual metric")
		t.Fail()
	}

	res, _ = http.Get(ts.URL)
	mBody, _ = io.ReadAll(res.Body)
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
}
