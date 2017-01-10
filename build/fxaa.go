package main

import (
	"fmt"
	"log"
	"os/exec"
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

func main() {
	runCommand("/usr/bin/clang", "-E", "-P", "-DFXAA_PC=1", "-DFXAA_QUALITY__PRESET=12", "-DFXAA_GREEN_AS_LUMA=1", "-DFXAA_GLSL_130=1", "-o", "fxaa.fsh", "build/Fxaa3_11_custom.h")
	runCommand("./build/clang-format", "-i", "fxaa.fsh")
}
