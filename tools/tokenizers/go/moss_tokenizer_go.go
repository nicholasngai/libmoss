package main

import (
	"fmt"
	"go/scanner"
	"go/token"
	"io/ioutil"
	"os"
)

func panicIfErr(err error) {
	if err != nil {
		panic(err)
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("usage: " + os.Args[0] + " file")
		os.Exit(-1)
	}

	filename := os.Args[1]
	data, err := ioutil.ReadFile(filename)
	panicIfErr(err)

	var s scanner.Scanner

	fset := token.NewFileSet()
	file := fset.AddFile(filename, fset.Base(), len(data))
	s.Init(file, data, nil, 0)

	for {
		_, tok, _ := s.Scan()
		if tok == token.EOF {
			break
		}
		fmt.Println(int(tok))
	}
}
