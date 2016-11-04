package xbinmap

import (
	"github.com/jurgen-kluft/xcode/denv"
	"github.com/jurgen-kluft/xentry/package"
	"github.com/jurgen-kluft/xunittest/package"
)

// GetPackage returns the package object of 'xbinmap'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := xunittest.GetPackage()
	entrypkg := xentry.GetPackage()

	// The main (xbinmap) package
	mainpkg := denv.NewPackage("xbinmap")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(entrypkg)

	// 'xbinmap' library
	mainlib := denv.SetupDefaultCppLibProject("xbinmap", "github.com\\jurgen-kluft\\xbinmap")
	mainlib.Dependencies = append(mainlib.Dependencies, unittestpkg.GetMainLib())

	// 'xbinmap' unittest project
	maintest := denv.SetupDefaultCppTestProject("xbinmap_test", "github.com\\jurgen-kluft\\xbinmap")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, entrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
