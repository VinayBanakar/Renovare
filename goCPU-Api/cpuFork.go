package main

import (
	"os"
	"syscall"
	"time"
	"fmt"
)

const (
	CHILD_ARG = "child"
)

func main() {
	if len(os.Args) >= 2 && os.Args[1] == CHILD_ARG {
		fmt.Printf("Child PID %v\n", os.Getpid())
		<-time.After(3 * time.Second)
		fmt.Printf("Child %v dies now.\n", os.Getpid())
		return
	}
	fmt.Printf("Parent PID %v\n", os.Getpid())
	pid, err := syscall.ForkExec(os.Args[0], []string{os.Args[0], CHILD_ARG}, &syscall.ProcAttr{Files: []uintptr{0, 1, 2}})
	if err != nil {
		panic(err.Error())
	}
	proc, err := os.FindProcess(pid)
	if err != nil {
		panic(err.Error())
	}
	state, err := proc.Wait()
	if err != nil {
		panic(err.Error())
	}
	println("string:", state.String())
	fmt.Printf("Parent %v dies now.\n", os.Getpid())
}