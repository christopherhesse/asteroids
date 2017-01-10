package main

import (
	"bufio"
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
)

// https://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format

var dataFile io.Writer
var srcFile io.Writer

const chars = 128

type FontWeight int

const (
	weightMedium FontWeight = iota
	weightBold
)

type Font struct {
	Width  int
	Height int
	Weight FontWeight
	Glyphs map[rune]Glyph
}

type Glyph struct {
	Encoding rune
	Bitmap   []bool
}

func visit(filePath string, f os.FileInfo, err error) error {
	if f.IsDir() {
		return nil
	}

	if path.Ext(f.Name()) != ".bdf" {
		return nil
	}

	contents, err := ioutil.ReadFile(filePath)
	if err != nil {
		return err
	}

	font := Font{Glyphs: map[rune]Glyph{}}
	glyph := Glyph{}

	scanner := bufio.NewScanner(bytes.NewReader(contents))

	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, " ")
		switch parts[0] {
		case "STARTCHAR":
			glyph = Glyph{}
		case "WEIGHT_NAME":
			switch parts[1] {
			case `"Bold"`:
				font.Weight = weightBold
			case `"Medium"`:
				font.Weight = weightMedium
			default:
				return errors.New("invalid weight")
			}
		case "ENCODING":
			encoding, err := strconv.Atoi(parts[1])
			if err != nil {
				return err
			}
			glyph.Encoding = rune(encoding)
		case "FONTBOUNDINGBOX":
			font.Width, err = strconv.Atoi(parts[1])
			if err != nil {
				return err
			}
			font.Height, err = strconv.Atoi(parts[2])
			if err != nil {
				return err
			}
		case "BITMAP":
			for i := 0; i < font.Height; i++ {
				if !scanner.Scan() {
					break
				}
				b, err := hex.DecodeString(scanner.Text())
				if err != nil {
					return err
				}
				for j := 0; j < font.Width; j++ {
					offset := 7 - uint(j%8)
					bit := int(b[j/8]) & (1 << offset) >> offset
					glyph.Bitmap = append(glyph.Bitmap, bit == 1)
				}
			}
		case "ENDCHAR":
			font.Glyphs[glyph.Encoding] = glyph
		}
	}

	if err := scanner.Err(); err != nil {
		return err
	}

	bytes := make([]bool, font.Width*font.Height*chars)

	for r := 0; r < chars; r++ {
		g, ok := font.Glyphs[rune(r)]
		if !ok {
			continue
		}

		for y := 0; y < font.Height; y++ {
			for x := 0; x < font.Width; x++ {
				if g.Bitmap[y*font.Width+x] {
					offset := (font.Height-y-1)*font.Width*chars + r*font.Width + x
					bytes[offset] = true
				}
			}
		}
	}

	texture := make([]byte, font.Width*font.Height*chars*4)

	for i, b := range bytes {
		if b {
			offset := i * 4
			texture[offset] = 255
			texture[offset+1] = 255
			texture[offset+2] = 255
			texture[offset+3] = 255
		}
	}

	weight := ""

	switch font.Weight {
	case weightMedium:
		weight = "Medium"
	case weightBold:
		weight = "Bold"
	}

	name := fmt.Sprintf("Terminus%d%s", font.Height, weight)

	fmt.Fprintf(srcFile, "Font %s = {\n", name)
	fmt.Fprintf(srcFile, "  .Width = %d,\n", font.Width)
	fmt.Fprintf(srcFile, "  .Height = %d,\n", font.Height)
	fmt.Fprintf(srcFile, "  .Chars = %d,\n", chars)
	fmt.Fprintf(srcFile, "  .Data = %sData,\n", name)
	fmt.Fprintf(srcFile, "};\n\n")

	fmt.Fprintf(dataFile, "uint8_t %sData[] = {", name)
	for i, b := range texture {
		if i == len(texture)-1 {
			fmt.Fprintf(dataFile, "0x%02x", b)
		} else {
			fmt.Fprintf(dataFile, "0x%02x, ", b)
		}
	}
	fmt.Fprintf(dataFile, "};\n")

	return nil
}

func run() error {
	var err error

	dataFile, err = os.Create(os.Args[1] + "/font.data")
	if err != nil {
		return err
	}

	srcFile, err = os.Create(os.Args[1] + "/font.c")
	if err != nil {
		return err
	}

	fmt.Fprintf(srcFile,
		`#include <stdlib.h>
#include "font.h"
#include "font.data"

#if INTERFACE

typedef struct {
    int Width;
    int Height;
    int Chars;
		uint8_t *Data;
} Font;

#endif

`)

	if err := filepath.Walk("build/font", visit); err != nil {
		return err
	}
	return nil
}

func main() {
	if err := run(); err != nil {
		log.Fatal(err)
	}
}
