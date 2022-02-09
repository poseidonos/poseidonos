package cmd

import "sync"

func Run() {
	var wait sync.WaitGroup
	wait.Add(3)

	go runSubscriber()
	go runProvider()
	go runExpiryManager()

	wait.Wait()
}
