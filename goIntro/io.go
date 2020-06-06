package main

import (
	"fmt"
	"io/ioutil"
)

func main() {
	
	mydata := []byte("all my data I want to write to a file")
	err := ioutil.WriteFile("myfile.data", mydata, 0777)
	if err != nil {
		// print it out
		fmt.Println(err)
	}

	data, err := ioutil.ReadFile("myfile.data")
    if err != nil {
        fmt.Println(err)
    }

    fmt.Print(string(data))
}