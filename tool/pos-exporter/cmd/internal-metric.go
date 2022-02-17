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

type HistogramMetric struct {
	name        string
	labels      *map[string]string
	bucketRange []int64
	bucketCount []uint64
	sum         int64
	totalCount  uint64
}
