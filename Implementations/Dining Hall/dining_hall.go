package main

import (
  "fmt"
  "sync"
  "time"
)

const numStudents = 5
const totalDines = 100000

type DiningHall struct {
  num_dining int
  num_ready_to_leave int
  mutex sync.Mutex
  cond *sync.Cond
}

type Student struct {
  index int
}

func (hall *DiningHall) is_student_waiting_for_us() bool {
  return hall.num_dining == 2 && hall.num_ready_to_leave == 1
}

func (hall *DiningHall) is_one_student_still_dining() bool {
  return hall.num_dining == 1 && hall.num_ready_to_leave == 1
}

func (hall *DiningHall) is_everyone_ready_to_leave() bool {
  return hall.num_dining == 0 && hall.num_ready_to_leave == 2
}

func (student *Student) dine(hall *DiningHall, num_dines *int, dining *sync.WaitGroup) {
  hall.mutex.Lock()
  // fmt.Printf("%v is dining\n", student.index)
  // time.Sleep(2 * time.Second)
  hall.num_dining++
  *num_dines++

  if *num_dines == totalDines {
    dining.Done()
  }
  
  if hall.is_student_waiting_for_us() {
    // Allow the person who's done eating to leave since either a new person is here, or we're done eating
    hall.cond.Signal()
    hall.num_ready_to_leave--
  }
  hall.mutex.Unlock()
}

func (student *Student) ready_to_leave(hall *DiningHall) {
  hall.mutex.Lock()
  // fmt.Printf("%v is ready to leave\n", student.index)
  hall.num_dining--
  hall.num_ready_to_leave++

  if hall.is_one_student_still_dining() {
    // fmt.Printf("%v is waiting\n", student.index)
    hall.cond.Wait()
  } else if hall.is_everyone_ready_to_leave() {
    // Person is waiting for us in the if branch above but we finished eating and signalled before they could call Wait()
    hall.num_ready_to_leave -= 2
    hall.cond.Signal()
  } else {
    hall.num_ready_to_leave--
    hall.cond.Signal()
  }
  hall.mutex.Unlock()
}

func main() {
  start := time.Now()
  var dining sync.WaitGroup

  hall := DiningHall{num_dining: 0}
  hall.cond = sync.NewCond(&hall.mutex)
  num_dines := 0

  dining.Add(1)
  for i := 0; i < numStudents; i++ {
    student := Student{i}

    go func(student *Student, hall *DiningHall, dining *sync.WaitGroup, num_dines *int) {
      for {
        student.dine(hall, num_dines, dining)
        student.ready_to_leave(hall)

        // fmt.Printf("%v is leaving\n", student.index)
      }
    }(&student, &hall, &dining, &num_dines)
  }

  dining.Wait()
  fmt.Printf("Time difference = %s\n", time.Since(start))
}
