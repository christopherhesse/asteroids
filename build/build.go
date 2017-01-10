package main

import (
	"fmt"
	"log"
	"os/exec"
	"path/filepath"
)

func runCommand(name string, args ...string) {
	cmd := exec.Command(name, args...)
	output, err := cmd.CombinedOutput()
	if err == nil {
		fmt.Println("success", cmd.Args)
	} else {
		fmt.Println("failure", cmd.Args)
		fmt.Println(string(output))
		log.Fatal(err)
	}
}

func clangFormat(pattern string) {
	matches, err := filepath.Glob(pattern)
	if err != nil {
		log.Fatal(err)
	}

	args := append([]string{"-i"}, matches...)
	runCommand("./build/clang-format", args...)
}

func main() {
	runCommand("/usr/local/go/bin/go", "run", "build/convertshaders.go", "src")
	runCommand("/usr/local/go/bin/go", "run", "build/model.go", "src")
	// runCommand("/usr/local/bin/go", "run", "build/font.go", "src")
	runCommand("/usr/local/go/bin/go", "run", "build/makematrix.go", "src/matrix.c")
	matches, err := filepath.Glob("src/*.c")
	if err != nil {
		log.Fatal(err)
	}

	runCommand("./build/makeheaders", matches...)

	clangFormat("src/*.c")
	clangFormat("src/*.vsh")
	clangFormat("src/*.fsh")
}
