package main

import (
	"github.com/jurgen-kluft/xbinmap/package"
	"github.com/jurgen-kluft/xcode"
)

func main() {
	xcode.Generate(xbinmap.GetPackage())
}
