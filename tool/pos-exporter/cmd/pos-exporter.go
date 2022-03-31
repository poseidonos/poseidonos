package cmd

import "sync"

var (
	mutex = &sync.Mutex{}
)

func Run() {
	var wait sync.WaitGroup
	wait.Add(3)
	
	parseCustomLabel()

	go runSubscriber()
	go runProvider()
	go runExpiryManager()

	wait.Wait()
}
