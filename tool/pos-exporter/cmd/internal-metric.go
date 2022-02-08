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
	name   string
	labels *map[string]string

	underflowCount uint64
	overflowCount  uint64
	lowerBound     int64
	upperBound     int64
	zeroIndex      int32
	bucketScale    int32
	scaleType      int32
	bucketRange    []int64
	bucketCount    []uint64
}
