#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <chrono>

int constexpr NUM_MEN = 6;
int constexpr NUM_WOMEN = 6;
int constexpr TOTAL_BATHROOM_VISITS = 100000;

struct Bathroom {
  int num_men = 0;
  int num_women = 0;
  std::mutex mutex;
  std::condition_variable cond;
};

struct Man {
  int index;
};

struct Woman {
  int index;
};

int main() {
  auto begin = std::chrono::steady_clock::now();

  Bathroom bathroom;
  auto num_visits = 0;
  std::vector<std::thread> threads(NUM_MEN + NUM_WOMEN);
  for (auto i = 0; i < NUM_MEN; ++i) {
    Man man{i};

    threads[i] = std::thread ([&bathroom, &num_visits, man] () mutable {
      while (true) {
        std::unique_lock<std::mutex> lock(bathroom.mutex);
        // std::cout << "Checking to see if " << man.index << " man can go" << std::endl;
        // Check if the bathroom can be used
        if (bathroom.num_men == 3 || bathroom.num_women > 0) {
          if (num_visits >= TOTAL_BATHROOM_VISITS) {
            return;
          }
          bathroom.cond.wait(lock);
        }

        // Use the bathroom
        // std::cout << man.index << " is going as man" << std::endl;
        ++bathroom.num_men;
        ++num_visits;

        if (num_visits >= TOTAL_BATHROOM_VISITS) {
          return;
        }

        // broadcast that bathroom can be used
        --bathroom.num_men;
        bathroom.cond.notify_all();
      }
    });
  }

  for (auto i = 0; i < NUM_WOMEN; ++i) {
    Woman woman{i};

    threads[NUM_MEN + i] = std::thread ([&bathroom, &num_visits, woman] () mutable {
      while (true) {
        std::unique_lock<std::mutex> lock(bathroom.mutex);
        // std::cout << "Checking to see if " << woman.index << " woman can go" << std::endl;
        // Check if the bathroom can be used
        if (bathroom.num_women == 3 || bathroom.num_men > 0) {
          if (num_visits >= TOTAL_BATHROOM_VISITS) {
            return;
          }
          bathroom.cond.wait(lock);
        }

        // Use the bathroom
        // std::cout << woman.index << " is going as woman" << std::endl;
        ++bathroom.num_women;
        ++num_visits;

        if (num_visits >= TOTAL_BATHROOM_VISITS) {
          return;
        }

        // broadcast that bathroom can be used
        --bathroom.num_women;
        bathroom.cond.notify_all();
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
}
