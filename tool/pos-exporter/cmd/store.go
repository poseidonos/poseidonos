package cmd

import (
	"github.com/prometheus/client_golang/prometheus"
)

var (
	counters     = make(map[string]*prometheus.CounterVec)
	gauges       = make(map[string]*prometheus.GaugeVec)
	histogramMap = make(map[string]*POSHistogramCollector)
)

func getLabelKeys(labels *map[string]string) *[]string {
	keys := make([]string, len(*labels))
	i := 0
	for key := range *labels {
		keys[i] = key
		i++
	}

	return &keys
}

func addCounter(in *CounterMetric) {
	vec, exists := counters[in.name]
	if !exists {
		vec = prometheus.NewCounterVec(
			prometheus.CounterOpts{Name: in.name, Help: "POS Counter Metric"},
			*getLabelKeys(in.labels),
		)
		prometheus.MustRegister(vec)
		counters[in.name] = vec
	}
	vec.With(*in.labels).Add(float64(in.value))

	touchExpiryVec(&in.name, in.labels)
}

func addGauge(in *GaugeMetric) {
	vec, exists := gauges[in.name]
	if !exists {
		vec = prometheus.NewGaugeVec(
			prometheus.GaugeOpts{Name: in.name, Help: "POS Gauge Metric"},
			*getLabelKeys(in.labels),
		)
		prometheus.MustRegister(vec)
		gauges[in.name] = vec
	}
	vec.With(*in.labels).Set(float64(in.value))

	touchExpiryVec(&in.name, in.labels)
}

func addHistogram(in *HistogramMetric) {
	histVector, exists := histogramMap[in.name]
	if !exists {
		histVector = NewPOSHistogramCollector(in.name, getSortedKeyFromLabelMap(in.labels))
		histogramMap[in.name] = histVector
		prometheus.MustRegister(histVector)
	}

	hist := histVector.FindHistogram(in.labels)
	if hist == nil {
		hist = NewPOSHistogram(len(in.bucketCount))
		histVector.AddHistogram(in.labels, hist)
	}

	// Update Label Key, Value
	var labelKeys = getSortedKeyFromLabelMap(in.labels)
	var labelValues = make([]string, len(labelKeys))
	for idx := range labelKeys {
		labelValues[idx] = (*in.labels)[labelKeys[idx]]
	}
	hist.UpdateLabelKey(labelKeys)
	hist.UpdateLabelValues(labelValues)
	hist.UpdateHistogram(in)

	touchExpiryVec(&in.name, in.labels)
}
