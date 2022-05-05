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
	data, err := ioutil.ReadAll(os.Stdin)
	panicIfErr(err)

	var s scanner.Scanner

	fset := token.NewFileSet()
	file := fset.AddFile("file.go", fset.Base(), len(data))
	s.Init(file, data, nil, 0)

	for {
		_, tok, _ := s.Scan()
		if tok == token.EOF {
			break
		}
		fmt.Println(int(tok))
	}
}
