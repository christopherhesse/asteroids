// GOBIN=$PWD go install convertshaders.go
package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	root := os.Args[1]

	outfile, err := os.Create(filepath.Join(root, "shader.c"))
	if err != nil {
		log.Fatal(err)
	}
	defer outfile.Close()

	walkFunc := func(path string, info os.FileInfo, err error) error {
		base := filepath.Base(path)
		ext := filepath.Ext(path)

		if ext == ".vsh" || ext == ".fsh" {
			infile, err := os.Open(path)
			if err != nil {
				log.Fatal(err)
			}
			defer infile.Close()

			lines := []string{}
			scanner := bufio.NewScanner(infile)
			for scanner.Scan() {
				lines = append(lines, `"`+scanner.Text()+`\n"`)
			}

			lines = append(lines, `"";`)

			if err := scanner.Err(); err != nil {
				return err
			}

			name := base[:len(base)-len(ext)]
			name = strings.ToUpper(string(name[0])) + name[1:]

			kind := ""
			if ext == ".vsh" {
				kind = "Vertex"
			} else if ext == ".fsh" {
				kind = "Fragment"
			} else {
				log.Fatal("unrecognized kind")
			}

			fmt.Fprintln(outfile, "const char *const "+name+kind+`Source = ""`)

			fmt.Fprintln(outfile, "#if TARGET_OS_IPHONE")
			fmt.Fprintln(outfile, `"#version 300 es\n"`)
			fmt.Fprintln(outfile, "#else")
			fmt.Fprintln(outfile, `"#version 330\n"`)
			fmt.Fprintln(outfile, "#endif")

			for _, line := range lines {
				fmt.Fprintln(outfile, line)
			}
		}
		return nil
	}

	filepath.Walk(root, walkFunc)
}
