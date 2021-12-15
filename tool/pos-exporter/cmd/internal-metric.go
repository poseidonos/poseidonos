package cmd

type CounterMetric struct {
	name   string
	labels *map[string]string
	value  uint64
}

type GaugeMetric struct {
	name   string
	labels *map[string]string
	value  int64
}
