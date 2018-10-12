package main

import (
  "fmt"
  "sync"
  "time"
)

const numSavages = 5
const totalServings = 3000
const totalRefills = 3

type Pot struct {
  mutex sync.Mutex
  remaining_servings int

  pot_empty chan bool
  pot_full chan bool
}

type Savage struct {
  index int
}

func main() {
  start := time.Now()
  var eating sync.WaitGroup

  pot_empty := make(chan bool)
  pot_full := make(chan bool)
  pot := Pot{remaining_servings: totalServings, pot_empty: pot_empty, pot_full: pot_full}

  eating.Add(1)
  go func(pot *Pot, eating *sync.WaitGroup) {
    num_refills := 0
    for {
      <- pot.pot_empty
      pot.pot_full <- true
      num_refills++

      if num_refills == totalRefills {
        eating.Done()
      }
    }
  }(&pot, &eating)

  for i := 0; i < numSavages; i++ {
    savage := Savage{i}

    go func(savage *Savage, pot *Pot, eating *sync.WaitGroup) {
      for {
        pot.mutex.Lock()
        if pot.remaining_servings == 0 {
          // fmt.Printf("Telling cook to refill pot %v from %v\n", pot.remaining_servings, savage.index)
          pot.pot_empty <- true
          <- pot.pot_full
          pot.remaining_servings = totalServings
        }
        pot.remaining_servings--
        // time.Sleep(5 * time.Millisecond)
        // fmt.Printf("Remaining servings left in pot are %v from %v savage\n", pot.remaining_servings, savage.index)
        pot.mutex.Unlock()
      }
    }(&savage, &pot, &eating)
  }

  eating.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
