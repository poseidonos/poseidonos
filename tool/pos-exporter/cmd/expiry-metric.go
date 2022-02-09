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
		_runExpiryManager()
		time.Sleep((time.Duration(valid_duration_second + 1)) * time.Second)
	}
}

func _runExpiryManager() {
	for name := range expiryVec {
		for key, _ := range expiryVec[name] {

			if (time.Now().Unix() - expiryVec[name][key]) >= valid_duration_second {

				label_kv := parseExpiryVecKey(&key)
				vec, exists := counters[name]

				if exists {
					vec.Delete(*label_kv)
				} else {
					vec, exists := gauges[name]
					if exists {
						vec.Delete(*label_kv)
					} else {
						fmt.Println("In the metric expiration process, unknown metric(%s) was accessed", name)
					}
				}
				delete(expiryVec[name], key)
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
