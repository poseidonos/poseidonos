package cmd

import (
	"github.com/prometheus/client_golang/prometheus"
)

var (
	counters = make(map[string]*prometheus.CounterVec)
	gauges   = make(map[string]*prometheus.GaugeVec)
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

}
