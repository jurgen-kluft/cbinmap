package main

import (
	cpkg "github.com/jurgen-kluft/cbinmap/package"
	ccode "github.com/jurgen-kluft/ccode"
)

func main() {
	ccode.Init()
	ccode.Generate(cpkg.GetPackage())
}
