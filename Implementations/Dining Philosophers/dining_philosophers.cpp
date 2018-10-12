#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <chrono>

int constexpr NUM_PHILOSOPHERS = 5;
int constexpr TOTAL_BITES_EATEN = 3000;

struct Fork {
  int index;
  std::mutex mutex;

  Fork &operator=(const Fork &f) {
      index = f.index;
      return *this;
  }
};

class Philosopher {
  int bites_eaten = 0;

  public:
    int index = 0;
    Fork& dominant_hand;
    Fork& other_hand;

    Philosopher(int index1, Fork& dominant_hand1, Fork& other_hand1): dominant_hand(dominant_hand1), other_hand(other_hand1) {
      index = index1;
    }

    void think() {
      // usleep(2e+6);
      // std::cout << index << " is thinking" << std::endl;
    }

    void get_forks() {
      dominant_hand.mutex.lock();
      // std::cout << index << " is waiting for left fork" << std::endl;
      other_hand.mutex.lock();
      // std::cout << index << " is waiting for right fork" << std::endl;
    }

    void eat() {
      bites_eaten++;
      // std::cout << index << " has eaten " << bites_eaten << " times" << std::endl;
    }

    void put_forks() {
      dominant_hand.mutex.unlock();
      other_hand.mutex.unlock();
      // std::cout << index << " has put forks down" << std::endl;
    }

    bool is_done_eating() {
      return bites_eaten == TOTAL_BITES_EATEN;
    }
};

int main() {
  auto begin = std::chrono::steady_clock::now();

  // Create the forks for the philosophers to use
  std::vector<Fork> forks(NUM_PHILOSOPHERS);
  for (auto i = 0; i < NUM_PHILOSOPHERS; ++i) {
    Fork f;
    f.index = i;
    forks[i] = f;
  }

  std::vector<std::thread> threads;
  for (auto i = 0; i < NUM_PHILOSOPHERS; ++i) {
    Philosopher p(i, i == 0 ? forks[(i + 1) % NUM_PHILOSOPHERS] : forks[i], i == 0 ? forks[i] : forks[(i + 1) % NUM_PHILOSOPHERS]);

    threads.push_back(std::thread ([p] () mutable {
        while (!p.is_done_eating()) {
          p.think();
          p.get_forks();
          p.eat();
          p.put_forks();
        }
    }));
  }

  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "µs" << std::endl;
}
