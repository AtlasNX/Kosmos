package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
)

func main() {
	var modules []string

	fdr, _ := ioutil.ReadDir(".")
	for _, item := range fdr {
		if item.IsDir() {
			modules = append(modules, item.Name())
		}
	}

	fmt.Println(`
		https://github.com/AtlasNX/Kosmos

 ____  __.
|    |/ _|____  ______ _____   ____  ______
|      < /  _ \/  ___//     \ /  _ \/  ___/
|    |  (  <_> )___ \|  Y Y  (  <_> )___ \
|____|__ \____/____  >__|_|  /\____/____  >
        \/         \/      \/           \/


	By: @AtlasNX (AtlasNX/Kosmos on Github)`)

	if _, err := os.Stat("compiled"); !os.IsNotExist(err) {
		os.RemoveAll("compiled")
	}

	for _, item := range modules {
		dir(item, "compiled")
	}
}

func file(src, dst string) error {
	var err error
	var srcfd *os.File
	var dstfd *os.File
	var srcinfo os.FileInfo

	if srcfd, err = os.Open(src); err != nil {
		return err
	}
	defer srcfd.Close()

	if dstfd, err = os.Create(dst); err != nil {
		return err
	}
	defer dstfd.Close()

	if _, err = io.Copy(dstfd, srcfd); err != nil {
		return err
	}
	if srcinfo, err = os.Stat(src); err != nil {
		return err
	}
	return os.Chmod(dst, srcinfo.Mode())
}

func dir(src string, dst string) error {
	var err error
	var fds []os.FileInfo
	var srcinfo os.FileInfo

	if srcinfo, err = os.Stat(src); err != nil {
		return err
	}

	if err = os.MkdirAll(dst, srcinfo.Mode()); err != nil {
		return err
	}

	if fds, err = ioutil.ReadDir(src); err != nil {
		return err
	}
	for _, fd := range fds {
		srcfp := path.Join(src, fd.Name())
		dstfp := path.Join(dst, fd.Name())

		if fd.IsDir() {
			if err = dir(srcfp, dstfp); err != nil {
				fmt.Println(err)
			}
		} else {
			if err = file(srcfp, dstfp); err != nil {
				fmt.Println(err)
			}
		}
	}
	return nil
}
