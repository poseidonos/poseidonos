package cmd

import (
	"sort"
	"strings"
)

func MakeLabelExporterFormat(l *map[string]string) string {

	keys := getLabelKeys(l)
	sort.Strings(*keys)

	kv_slice := make([]string, len(*keys))

	for i, key := range *keys {
		kv_slice[i] = key + "=" + "\"" + (*l)[key] + "\""
	}

	return strings.Join(kv_slice, ",")
}
