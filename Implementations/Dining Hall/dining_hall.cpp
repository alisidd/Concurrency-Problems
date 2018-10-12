#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <chrono>

int constexpr NUM_STUDENTS = 5;
int constexpr TOTAL_DINES = 100000;

struct DiningHall {
  int num_dining = 0;
  int num_ready_to_leave = 0;
  std::mutex mutex;
  std::condition_variable cond;
};

struct Student {
  int index;
};

bool is_student_waiting_for_us(const DiningHall& hall) {
  return hall.num_dining == 2 && hall.num_ready_to_leave == 1;
}

bool is_one_student_still_dining(const DiningHall& hall) {
  return hall.num_dining == 1 && hall.num_ready_to_leave == 1;
}

bool is_everyone_ready_to_leave(const DiningHall& hall) {
  return hall.num_dining == 0 && hall.num_ready_to_leave == 2;
}

int main() {
  auto begin = std::chrono::steady_clock::now();

  DiningHall hall;
  auto num_dines = 0;
  std::vector<std::thread> threads(NUM_STUDENTS);
  for (auto i = 0; i < NUM_STUDENTS; ++i) {
    Student student{i};

    threads[i] = std::thread ([&hall, &num_dines, student] () mutable {
      while (true) {
        std::unique_lock<std::mutex> lock(hall.mutex);

        // std::cout << student.index << " is dining" << std::endl;
        ++hall.num_dining;
        ++num_dines;

        if (num_dines >= TOTAL_DINES) {
          return;
        }

        if (is_student_waiting_for_us(hall)) {
          // Allow the person who's done eating to leave since either a new person is here, or we're done eating
          hall.cond.notify_all();
          --hall.num_ready_to_leave;
        }

        // std::cout << student.index << " is ready to leave" << std::endl;
        --hall.num_dining;
        ++hall.num_ready_to_leave;

        if (is_one_student_still_dining(hall)) {
          // std::cout << student.index << " is waiting" << std::endl;
          hall.cond.wait(lock);
        } else if (is_everyone_ready_to_leave(hall)) {
          // Person is waiting for us in the if branch above but we finished eating and signalled before they could call Wait()
          hall.num_ready_to_leave -= 2;
          hall.cond.notify_all();
        } else {
          --hall.num_ready_to_leave;
          hall.cond.notify_all();
        }

        // std::cout << student.index << " is leaving" << std::endl;
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
}
