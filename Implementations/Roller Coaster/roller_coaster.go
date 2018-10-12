package main

import (
  "fmt"
  "sync"
  "time"
)

const totalCapacity = 10
const numPassengers = 12
const totalRides = 1000

type RollerCoaster struct {
  capacity int
  board_channel chan bool
  unboard_channel chan int
  completed_rides int
}

type Passenger struct {
  index int
  is_riding bool
  board_channel chan bool
  unboard_channel chan int
}

func (r *RollerCoaster) load() {
  // fmt.Println("Loading!")
  for i := 0; i < r.capacity; i++ {
    r.board_channel <- true
  }
}

func (r *RollerCoaster) run() {
  // time.Sleep(5 * time.Millisecond)
  // fmt.Println("Ride finished!")
}

func (r *RollerCoaster) unload(riding_open *sync.WaitGroup) {
  r.completed_rides++
  i := 0
  for {
    <-r.unboard_channel
    i++
    // fmt.Printf("%v is unboarding\n", p)
    if i == r.capacity {
      break
    }
	}
  if r.completed_rides == totalRides {
    riding_open.Done()
  }
}

func (p *Passenger) board() {
  p.is_riding = <-p.board_channel
  // fmt.Printf("%v is boarding\n", p.index)
}

func (p *Passenger) unboard() {
  if p.is_riding {
    p.is_riding = false
    p.unboard_channel <- p.index
  }
}

func main() {
  start := time.Now()
  board_channel := make(chan bool)
  unboard_channel := make(chan int)
  var riding_open sync.WaitGroup

  riding_open.Add(1)

  r := RollerCoaster{capacity: totalCapacity, board_channel: board_channel, unboard_channel: unboard_channel}
  go func(r *RollerCoaster, riding_open *sync.WaitGroup) {
    for {
      r.load()
      r.run()
      r.unload(riding_open)
    }
  }(&r, &riding_open)

  for i := 0; i < numPassengers; i++ {
    p := Passenger{i, false, board_channel, unboard_channel}

    go func(p *Passenger) {
      for {
        p.board()
        p.unboard()
      }
    }(&p)
  }

  riding_open.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
