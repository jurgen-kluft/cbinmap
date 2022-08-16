package xbinmap

import (
	"github.com/jurgen-kluft/ccode/denv"
	"github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xentry/package"
)

// GetPackage returns the package object of 'xbinmap'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := xunittest.GetPackage()
	entrypkg := xentry.GetPackage()
	basepkg := xbase.GetPackage()

	// The main (xbinmap) package
	mainpkg := denv.NewPackage("xbinmap")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(entrypkg)
	mainpkg.AddPackage(basepkg)

	// 'xbinmap' library
	mainlib := denv.SetupDefaultCppLibProject("xbinmap", "github.com\\jurgen-kluft\\xbinmap")
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'xbinmap' unittest project
	maintest := denv.SetupDefaultCppTestProject("xbinmap_test", "github.com\\jurgen-kluft\\xbinmap")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, entrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
