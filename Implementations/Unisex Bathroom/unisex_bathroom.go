package main

import (
  "fmt"
  "sync"
  "time"
)

const numMen = 6
const numWomen = 6
const totalBathroomVisits = 100000

type Bathroom struct {
  num_men int
  num_women int
  mutex sync.Mutex
  cond *sync.Cond
}

type Man struct {
  index int
}

type Woman struct {
  index int
}

func main() {
  start := time.Now()
  var bathroom_visiting sync.WaitGroup

  bathroom := Bathroom{num_men: 0, num_women: 0}
  bathroom.cond = sync.NewCond(&bathroom.mutex)
  num_visits := 0

  bathroom_visiting.Add(1)

  for i := 0; i < numMen; i++ {
    man := Man{i}

    go func(man *Man, bathroom *Bathroom, bathroom_visitng *sync.WaitGroup, num_visits *int) {
      for {
        bathroom.mutex.Lock()
        // fmt.Printf("Checking to see if %v man can go\n", man.index)
        // Check if the bathroom can be used
        for bathroom.num_men == 3 || bathroom.num_women > 0 {
          bathroom.cond.Wait()
        }

        // Use the bathroom
        // fmt.Printf("%v is going as man\n", man.index)
        bathroom.num_men++
        *num_visits++
        if *num_visits == totalBathroomVisits {
          bathroom_visiting.Done()
        }
        bathroom.mutex.Unlock()

        // broadcast that bathroom can be used
        bathroom.mutex.Lock()
        bathroom.num_men--
        bathroom.cond.Signal()
        bathroom.mutex.Unlock()
      }
    }(&man, &bathroom, &bathroom_visiting, &num_visits)
  }

  for i := 0; i < numWomen; i++ {
    woman := Woman{i}

    go func(woman *Woman, bathroom *Bathroom, bathroom_visitng *sync.WaitGroup, num_visits *int) {
      for {
        bathroom.mutex.Lock()
        // fmt.Printf("Checking to see if %v woman can go\n", woman.index)
        // Check if the bathroom can be used
        for bathroom.num_women == 3 || bathroom.num_men > 0 {
          bathroom.cond.Wait()
        }

        // Use the bathroom
        // fmt.Printf("%v is going as woman\n", woman.index)
        bathroom.num_women++
        *num_visits++
        if *num_visits == totalBathroomVisits {
          bathroom_visiting.Done()
        }
        bathroom.mutex.Unlock()

        // broadcast that bathroom can be used
        bathroom.mutex.Lock()
        bathroom.num_women--
        bathroom.cond.Signal()
        bathroom.mutex.Unlock()
      }
    }(&woman, &bathroom, &bathroom_visiting, &num_visits)
  }

  bathroom_visiting.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
