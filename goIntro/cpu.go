package main

import (
	"fmt"
	"os"
	"time"
	"strconv"
)

func Spin(howlong int64) {
	t := time.Now().Unix()
	for (time.Now().Unix() - t) < howlong {}
}

func main(){
	k,_ := strconv.Atoi(os.Args[1])
	fmt.Printf("k: %v %T\n",k, k)
	for true {
		Spin(1)
		fmt.Printf("%v\n",k)
	}
}