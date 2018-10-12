package main

import (
  "fmt"
  "sync"
  "time"
)

const totalNumbers = 10000

func main() {
  start := time.Now()
  channel := make(chan int)
  var adding sync.WaitGroup

  adding.Add(1)

  go func(channel chan int) {
    for range channel {
      // fmt.Println(i)
	  }
    adding.Done()
  }(channel)

  go func(channel chan int) {
    for i := 0; i < totalNumbers; i++ {
      channel <- i
    }
    close(channel)
  }(channel)

  adding.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
