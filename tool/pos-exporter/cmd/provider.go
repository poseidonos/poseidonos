package cmd

import (
	"fmt"
	"log"
	"net/http"

	"github.com/prometheus/client_golang/prometheus/promhttp"
)

const (
	uri             = "/metrics"
	providerAddress = ":2112"
)

func runProvider() {
	fmt.Println("Run Provider!!")
	http.Handle(uri, promhttp.Handler())
	err := http.ListenAndServe(providerAddress, nil)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
}
