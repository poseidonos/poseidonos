package cmd

import (
	"fmt"
	"sort"
	"strings"
	"time"
)

var (
	expiryVec = make(map[string]map[string]int64)
)

const (
	valid_duration_second = int64(60)
	label_delimeter       = "$"
	kv_mapper             = "="
)

func touchExpiryVec(name *string, labels *map[string]string) {

	// Only performance-related metrics(publisher_name=air_delegator) will be expiration object.
	if (*labels)["publisher_name"] != "air_delegator" {
		return
	}

	keys := getLabelKeys(labels)

	sort.Strings(*keys)

	labelKeys := make([]string, 0)

	for _, key := range *keys {
		labelKeys = append(labelKeys, key+kv_mapper+(*labels)[key])
	}

	labelKey := strings.Join(labelKeys, label_delimeter)

	if expiryVec[*name] == nil {
		expiryVec[*name] = make(map[string]int64)
	}

	expiryVec[*name][labelKey] = time.Now().Unix()
}

func runExpiryManager() {
	for {
		mutex.Lock()
		_runExpiryManager()
		mutex.Unlock()
		time.Sleep((time.Duration(valid_duration_second + 1)) * time.Second)
	}
}

func _runExpiryManager() {
	for name := range expiryVec {
		for key, _ := range expiryVec[name] {

			if (time.Now().Unix() - expiryVec[name][key]) >= valid_duration_second {

				is_deleted := false
				label_kv := parseExpiryVecKey(&key)

				if vec, exists := counters[name]; exists {
					vec.Delete(*label_kv)
					is_deleted = true
				}

				if vec, exists := gauges[name]; exists {
					vec.Delete(*label_kv)
					is_deleted = true
				}

				if collector, exists := histogramMap[name]; exists {
					collector.RemoveHistogram(label_kv)
					is_deleted = true
				}

				if is_deleted {
					delete(expiryVec[name], key)
				} else {
					fmt.Printf("In the metric expiration process, unknown metric(%s) was accessed", name)
				}
			}
		}
	}
}

func parseExpiryVecKey(ls *string) *map[string]string {

	labels := strings.Split(*ls, label_delimeter)
	label_kv := make(map[string]string)

	for _, str := range labels {
		kv := strings.Split(str, kv_mapper)
		label_kv[kv[0]] = kv[1]
	}

	return &label_kv
}
