package main

import (
	"cli/cmd"
	"log"
	"os"

	"github.com/spf13/cobra/doc"
)

func main() {
	rootCmd := cmd.RootCmd
	header := &doc.GenManHeader{
		Title:   "MINE",
		Section: "3",
	}
	os.MkdirAll("../docs/manpage/", os.ModePerm)
	err := doc.GenManTree(rootCmd, header, "../docs/manpage")
	if err != nil {
		log.Fatal(err)
	}
}
