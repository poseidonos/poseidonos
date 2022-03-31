package cmd

import (
	"github.com/akamensky/argparse"
	"os"
	"fmt"
	"strings"
)

var (
	customLabelKey = ""
	customLabelVal = ""
)

func parseCustomLabel() {
	parser := argparse.NewParser("pos-exporter", "Export POS metrics to :2112")
	label := parser.String("l", "label", &argparse.Options{Required: false, Help: "add custom label - specify key and value separated by colon(:)"})
	
	if err := parser.Parse(os.Args); err != nil {
		fmt.Print(parser.Usage(err))
		os.Exit(1)
	}
	
	if *label != "" {
		if custom_kv := strings.Split(*label, ":");	len(custom_kv) != 2 {
			fmt.Printf("Invalid Label: [%s]\n", *label)
			os.Exit(1)
		} else {
			fmt.Printf("Add Custom Label with key(%s), value(%s)\n", custom_kv[0], custom_kv[1])
			setCustomLabel(custom_kv[0], custom_kv[1])
		}
	}
}

func setCustomLabel(key string, val string) {
	customLabelKey = key
	customLabelVal = val
}

func getCustomLabel() (string, string) {
	return customLabelKey, customLabelVal
}

func isValidCustomLabel() bool {
	if (customLabelKey != "") && (customLabelVal != "") {
		return true
	} else {
		return false
	}
}