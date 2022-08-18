package cbinmap

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	centry "github.com/jurgen-kluft/centry/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'cbinmap'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := cunittest.GetPackage()
	entrypkg := centry.GetPackage()
	basepkg := cbase.GetPackage()

	// The main (cbinmap) package
	mainpkg := denv.NewPackage("cbinmap")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(entrypkg)
	mainpkg.AddPackage(basepkg)

	// 'cbinmap' library
	mainlib := denv.SetupDefaultCppLibProject("cbinmap", "github.com\\jurgen-kluft\\cbinmap")
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'cbinmap' unittest project
	maintest := denv.SetupDefaultCppTestProject("cbinmap_test", "github.com\\jurgen-kluft\\cbinmap")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, entrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
