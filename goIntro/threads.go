package main

import (
	"fmt"
	"os"
	"strconv"
//	"time"
	"sync"
	"runtime/debug"
)

var wg sync.WaitGroup
var counter int = 0
var loops int
var mutex = &sync.Mutex{}

func worker() {
	var i int
	
	mutex.Lock()
	for i=0;i<loops; i++ {
		counter++
	}
	mutex.Unlock()
	
	wg.Done()
}

func main(){
	wg.Add(2)
	loops,_ = strconv.Atoi(os.Args[1])
	fmt.Printf("Initial value: %v \n", counter)
	go worker()
	go worker()

	wg.Wait()
	//time.Sleep(25 * time.Second)
	fmt.Printf("Final value: %v \n", counter)
	debug.PrintStack()
}