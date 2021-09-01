package main

import (
	"log"
	"os"

	"github.com/spf13/cobra/doc"

	"cli/cmd"
)

func main() {
	rootCmd := cmd.RootCmd
	// Disable auto-generated tag to prevent unneccessary modification of the docs
	rootCmd.DisableAutoGenTag = true

	os.MkdirAll("../docs/markdown/", os.ModePerm)
	err := doc.GenMarkdownTree(rootCmd, "../docs/markdown/")
	if err != nil {
		log.Fatal(err)
	}
}
