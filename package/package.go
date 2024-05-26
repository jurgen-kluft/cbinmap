package cbinmap

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	denv "github.com/jurgen-kluft/ccode/denv"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'cbinmap'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := cunittest.GetPackage()
	cbasepkg := cbase.GetPackage()

	// The main (cbinmap) package
	mainpkg := denv.NewPackage("cbinmap")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(cbasepkg)

	// 'cbinmap' library
	mainlib := denv.SetupDefaultCppLibProject("cbinmap", "github.com\\jurgen-kluft\\cbinmap")
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())

	// 'cbinmap' unittest project
	maintest := denv.SetupDefaultCppTestProject("cbinmap_test", "github.com\\jurgen-kluft\\cbinmap")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
