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

/*
	* Linear Type
	LowerBound -10, UpperBound +10, Scale 5 (linear type)
	Bucket Range [-10,-6] [-5,-1] [0,4] [5-9]
	Bucket Index    0        1      2     3

	* Exponential Type
	LowerBound -100, UpperBound +100, Scale 10 (linear type)
	Bucket Range   [-10^2,-10^1) [-10^1,-10^0) [-10^0,0) [0,10^0) [10^0,10^1) [10^1,10^2]
	Bucket Index    0                 1            2        3         4           5
	Note                                                  ZeroIdx
	UpperBound      -10^1         -10^0         0         1        10^1        10^2
*/
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
