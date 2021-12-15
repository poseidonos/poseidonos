package cmd

import "sync"

func Run() {
	var wait sync.WaitGroup
	wait.Add(2)

	go runSubscriber()
	go runProvider()

	wait.Wait()
}
