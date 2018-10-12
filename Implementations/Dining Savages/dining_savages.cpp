#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <chrono>

int constexpr NUM_SAVAGES = 5;
int constexpr TOTAL_SERVINGS = 3000;
int constexpr TOTAL_REFILLS = 5;

struct Pot {
  std::mutex mutex;
  std::condition_variable pot_full;
  std::condition_variable pot_empty;
  int remaining_servings = TOTAL_SERVINGS;
};

struct Savage {
  int index;
};

int main() {
  auto begin = std::chrono::steady_clock::now();

  Pot pot;
  std::vector<std::thread> threads(NUM_SAVAGES + 1);

  threads[0] = std::thread ([&pot] () mutable {
    auto num_refills = 0;
    while (true) {
      std::unique_lock<std::mutex> lock(pot.mutex);
      pot.pot_empty.wait(lock);
      pot.pot_full.notify_all();

      ++num_refills;

      if (num_refills == TOTAL_REFILLS) {
        break;
      }
    }
  });


  for (auto i = 0; i < NUM_SAVAGES; i++) {
    Savage savage {i};

    threads[i + 1] = std::thread ([savage, &pot] () mutable {
      while (true) {
        std::unique_lock<std::mutex> lock(pot.mutex);
        if (pot.remaining_servings == 0) {
          // std::cout << "Telling cook to refill pot " << pot.remaining_servings << " from " << savage.index << std::endl;
          pot.pot_empty.notify_one();
          pot.pot_full.wait(lock);
          pot.remaining_servings = TOTAL_SERVINGS;
        }

        --pot.remaining_servings;
        // usleep(1e+3);
        // std::cout << "Remaining servings left in pot are " << pot.remaining_servings << " from " << savage.index << std::endl;
      }
    });
  }

  threads[0].join();

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "µs" << std::endl;
}
