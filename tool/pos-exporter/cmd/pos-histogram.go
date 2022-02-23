package cmd

import (
	"fmt"
	"math"
	"sort"
	"strings"

	"github.com/prometheus/client_golang/prometheus"
)

type POSHistogram struct {
	bucket     map[float64]uint64
	count      uint64
	sum        float64
	labelKey   []string
	labelValue []string
}

type POSHistogramCollector struct {
	desc    *prometheus.Desc
	metrics map[string]*POSHistogram
}

func NewPOSHistogramCollector(name string, labelKey []string) *POSHistogramCollector {
	var posHistColector = POSHistogramCollector{
		metrics: make(map[string]*POSHistogram),
		desc:    prometheus.NewDesc(name, "POS Histogram", labelKey, nil),
	}
	return &posHistColector
}

func NewPOSHistogram(bucketSize int) *POSHistogram {
	var posHist = POSHistogram{
		bucket: make(map[float64]uint64, bucketSize),
	}
	return &posHist
}

func (collector *POSHistogramCollector) Describe(ch chan<- *prometheus.Desc) {
	ch <- collector.desc
}

func (collector *POSHistogramCollector) Collect(ch chan<- prometheus.Metric) {
	//ch <- prometheus.MustNewConstHistogram(collector.desc, collector.count, collector.sum, collector.bucket, collector.labelValue...)
	for _, metric := range collector.metrics {
		ch <- prometheus.MustNewConstHistogram(collector.desc, metric.count, metric.sum, metric.bucket, metric.labelValue...)
	}
}

func (collector *POSHistogramCollector) FindHistogram(labels *map[string]string) *POSHistogram {
	var mapIndex = getMapIndexString(labels)
	histogram, exists := collector.metrics[mapIndex]
	if !exists {
		return nil
	}
	return histogram
}

func (collector *POSHistogramCollector) AddHistogram(labels *map[string]string, histogram *POSHistogram) {
	var mapIndex = getMapIndexString(labels)
	collector.metrics[mapIndex] = histogram
}

func (collector *POSHistogramCollector) RemoveHistogram(labels *map[string]string) bool {
	var isRemoved = false
	var mapIndex = getMapIndexString(labels)

	if _, ok := collector.metrics[mapIndex]; ok {
		delete(collector.metrics, mapIndex)
		isRemoved = true
	}

	return isRemoved
}

func getSortedKeyFromLabelMap(labels *map[string]string) []string {
	var sortedKey = make([]string, 0, len(*labels))
	for key := range *labels {
		sortedKey = append(sortedKey, key)
	}
	sort.Strings(sortedKey)
	return sortedKey
}

func getMapIndexString(labels *map[string]string) string {
	var sortedKey = getSortedKeyFromLabelMap(labels)
	var strKvPair = make([]string, 0, len(sortedKey))

	for _, key := range sortedKey {
		var kvPair = fmt.Sprintf("%s-%s", key, (*labels)[key])
		strKvPair = append(strKvPair, kvPair)
	}

	return strings.Join(strKvPair, ",")
}

func CalculateUpperBound(in *HistogramMetric) []float64 {
	var bucketSize = len(in.bucketCount)

	// calculate upper bound
	var upperBoundArray = make([]float64, bucketSize)
	if in.scaleType == 0 {
		for bucketIndex := range upperBoundArray {
			var upperEq = (int32(bucketIndex) * int32(in.bucketScale)) + (int32(in.lowerBound) + int32(in.bucketScale))
			upperBoundArray[bucketIndex] = float64(upperEq)
		}
	} else if in.scaleType == 1 {
		var zeroIndex = int(in.zeroIndex)
		var pow = int(in.bucketScale)
		var curMulti = 1

		// zero index
		upperBoundArray[zeroIndex] = 1

		// (zeroindex+1 ~ bucektSize)
		curMulti = pow
		for bucketIndex := zeroIndex + 1; bucketIndex < bucketSize; bucketIndex++ {
			upperBoundArray[bucketIndex] = float64(curMulti)
			curMulti = curMulti * pow
		}

		// (0 ~ zeroIndex)
		for bucketIndex := 0; bucketIndex < zeroIndex; bucketIndex++ {
			var lowerBoundPowIdx = int(math.Abs(float64(bucketIndex-zeroIndex))) - 1
			var lowerBound = int(math.Pow(float64(in.bucketScale), float64(lowerBoundPowIdx)))
			var upperBound = lowerBound / int(in.bucketScale)
			upperBoundArray[bucketIndex] = float64(upperBound)
			if upperBound != 0 {
				upperBoundArray[bucketIndex] *= -1
			}
		}
	}

	return upperBoundArray
}

func (histogram *POSHistogram) UpdateHistogram(in *HistogramMetric) {
	var bucketSize = len(in.bucketCount)
	histogram.bucket = make(map[float64]uint64, bucketSize)
	histogram.count = 0
	histogram.sum = 0

	// accumulate sum (copy array)
	var accSum = make([]uint64, bucketSize)
	accSum[0] = in.bucketCount[0]
	for i := 1; i < bucketSize; i++ {
		accSum[i] = accSum[i-1] + in.bucketCount[i]
	}

	// get upper bound
	var upperBound = CalculateUpperBound(in)

	for idx, value := range accSum {
		histogram.bucket[upperBound[idx]] = value
	}

	// add overflow count ( for +inf )
	histogram.count += accSum[len(accSum)-1]
	histogram.count += in.overflowCount

}

func (histogram *POSHistogram) UpdateLabelKey(labelKey []string) {
	histogram.labelKey = make([]string, len(labelKey))
	for idx := range labelKey {
		histogram.labelKey[idx] = labelKey[idx]
	}
}

func (histogram *POSHistogram) UpdateLabelValues(labelValue []string) {
	histogram.labelValue = make([]string, len(labelValue))
	for idx := range labelValue {
		histogram.labelValue[idx] = labelValue[idx]
	}
}
