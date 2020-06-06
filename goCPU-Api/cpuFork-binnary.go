package main

import (
	"fmt"
	"syscall"
	"os"
	"os/exec"
//	"time"
)

func main() {

	cmd := "/Users/vinaybanakar/Documents/randProjects/OSTEP/code/goCPU-Api/myproc"
    binary, lookErr := exec.LookPath(cmd)
    if lookErr != nil {
        panic(lookErr)
    }
    fmt.Println(binary)

	fmt.Printf("The PID of parrent %v\n", os.Getpid())

	// fstdin, err1 := os.Create("/Users/vinaybanakar/Documents/randProjects/OSTEP/code/goCPU-Api/stdin")
    // fstdout, err2 := os.Create("/Users/vinaybanakar/Documents/randProjects/OSTEP/code/goCPU-Api/stdout")
    // fstderr, err3 := os.Create("/Users/vinaybanakar/Documents/randProjects/OSTEP/code/goCPU-Api/stderr")
    // if err1 != nil || err2 != nil || err3 != nil {
    //     fmt.Println(err1, err2, err3)
    //     panic("WOW")
    // }

	// procAttr := syscall.ProcAttr{
    //     Dir:   "/Users/vinaybanakar/Documents/randProjects/OSTEP/code/goCPU-Api/",
    //     Files: []uintptr{fstdin.Fd(), fstdout.Fd(), fstderr.Fd()},
    //     Env:   []string{""},
    //     Sys: &syscall.SysProcAttr{
    //         Foreground: false,
    //     },
	// }
	
	// pid, err := syscall.ForkExec(binary, nil, nil)
	pid, err := syscall.ForkExec(binary, nil, nil)
	fmt.Println("Forked child PID", pid, err)

    // time.Sleep(time.Second * 100)

	// rc, err := syscall.ForkExec()
	// if err!=nil {
	// 	fmt.Printf("Fork failed!!\n")
	// } else if rc == 0 {
	// 	fmt.Printf("The PID of forked child %v\n", os.Getpid())
	// } else {
	// 	fmt.Printf("Parrent of %v and my PID is %v", rc, os.Getpid())
	// }
}



