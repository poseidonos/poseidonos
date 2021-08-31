package main

import (
	"log"
	"os"

	"github.com/spf13/cobra/doc"

	"cli/cmd"
)

func main() {
	rootCmd := cmd.RootCmd
	os.MkdirAll("./markdown/", os.ModePerm)
	err := doc.GenMarkdownTree(rootCmd, "./markdown")
	if err != nil {
		log.Fatal(err)
	}
}
