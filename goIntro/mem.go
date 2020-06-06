package main

import (
	"fmt"
	"os"
	"time"
)

func Spin(howlong int64) {
	t := time.Now().Unix()
	for (time.Now().Unix() - t) < howlong {}
}

func main(){
	var  p *int
	x := 0
	p = &x
	fmt.Printf("Adress pointed to by p: %v \n", p)
	for true {
		Spin(1)
		*p = *p + 1
		fmt.Printf("(%v) p: %v x: %v p_value: %v p_addr: %v x_addr: %v\n", os.Getppid(), *p, x, p, &p, &x)
	}
}