package main

import (
  "fmt"
  "sync"
  "time"
)

const numPhilosophers = 5
const totalBitesEaten = 3000;

type Fork struct {
  index int
  mutex sync.Mutex
}

type Philosopher struct {
  index int
  dominant_hand *Fork
  other_hand *Fork
  bites_eaten int
}

func (p *Philosopher) think() {
  // time.Sleep(2 * time.Second)
  // fmt.Printf("%v is thinking\n", p.index)
}

func (p *Philosopher) get_forks() {
  // fmt.Printf("%v is waiting for left fork\n", p.index)
  p.dominant_hand.mutex.Lock()
  // fmt.Printf("%v is waiting for right fork\n", p.index)
  p.other_hand.mutex.Lock()
}

func (p *Philosopher) eat() {
  p.bites_eaten++
  // fmt.Printf("%v has eaten %v times\n", p.index, p.bites_eaten)
}

func (p *Philosopher) put_forks() {
  p.dominant_hand.mutex.Unlock()
  p.other_hand.mutex.Unlock()
  // fmt.Printf("%v has put forks down\n", p.index)
}

func (p *Philosopher) is_done_eating(dining *sync.WaitGroup) bool {
  if p.bites_eaten == totalBitesEaten {
    dining.Done()
  }

  return p.bites_eaten == totalBitesEaten
}

func main() {
  start := time.Now()
  var dining sync.WaitGroup

  // Create the forks for the philosophers to use
  forks := make([]Fork, 0, numPhilosophers)
  for i := 0; i < numPhilosophers; i++ {
    forks = append(forks, Fork{index: i})
  }

  for i := 0; i < numPhilosophers; i++ {
    dining.Add(1)
    p := Philosopher{i, &forks[i], &(forks[(i + 1) % numPhilosophers]), 0}
    if i == 0 {
      // Make one philosopher left handed
      p = Philosopher{i, &(forks[(i + 1) % numPhilosophers]), &forks[i], 0}
    }

    go func(p *Philosopher, dining *sync.WaitGroup) {
      for !p.is_done_eating(dining) {
        p.think()
        p.get_forks()
        p.eat()
        p.put_forks()
      }
    }(&p, &dining)
  }

  dining.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
