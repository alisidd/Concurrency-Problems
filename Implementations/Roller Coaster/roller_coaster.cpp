#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <chrono>

int constexpr TOTAL_CAPACITY = 10;
int constexpr NUM_PASSENGERS = 12;
int constexpr TOTAL_RIDES = 1000;

template<class item>
class channel {
private:
  std::list<item> queue;
  std::mutex m;
  std::condition_variable cv;
  bool closed;
public:
  channel() : closed(false) { }
  void close() {
    std::unique_lock<std::mutex> lock(m);
    closed = true;
    cv.notify_all();
  }
  bool is_closed() {
    std::unique_lock<std::mutex> lock(m);
    return closed;
  }
  void put(const item &i) {
    std::unique_lock<std::mutex> lock(m);
    if(closed)
      throw std::logic_error("put to closed channel");
    queue.push_back(i);
    cv.notify_one();
  }
  bool get(item &out, bool wait = true) {
    std::unique_lock<std::mutex> lock(m);
    if(wait)
      cv.wait(lock, [&](){ return closed || !queue.empty(); });
    if(queue.empty())
      return false;
    out = queue.front();
    queue.pop_front();
    return true;
  }
};

struct RollerCoaster {
  int capacity;
  channel<bool> board_channel;
  channel<int> unboard_channel;
  int completed_rides;
};

struct Passenger {
  int index;
};

int main() {
  auto begin = std::chrono::steady_clock::now();
  RollerCoaster rollerCoaster{.capacity = TOTAL_CAPACITY};

  std::vector<std::thread> threads(NUM_PASSENGERS + 1);
  threads[0] = std::thread ([&rollerCoaster] () mutable {
    while (true) {
      // std::cout << "Loading!" << std::endl;
      for (auto i = 0; i < rollerCoaster.capacity; i++) {
        rollerCoaster.board_channel.put(true);
      }

      ++rollerCoaster.completed_rides;
      // std::cout << "Ride finished!" << std::endl;

      int unboarded;
      auto i = 0;
      while (rollerCoaster.unboard_channel.get(unboarded)) {
        // std::cout << unboarded << " is unboarding" << std::endl;
        ++i;
        if (i == rollerCoaster.capacity) {
          break;
        }
      }

      if (rollerCoaster.completed_rides == TOTAL_RIDES) {
        break;
      }
    }
  });

  for (auto i = 0; i < NUM_PASSENGERS; ++i) {
    Passenger passenger{i};

    threads[i + 1] = std::thread ([&rollerCoaster, passenger] () mutable {
      while (true) {
        bool is_riding;
        if (rollerCoaster.board_channel.get(is_riding)) {
          // std::cout << passenger.index << " is boarding" << std::endl;
        };

        if (is_riding) {
          rollerCoaster.unboard_channel.put(passenger.index);
        }
      }
    });
  }

  threads[0].join();

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
}
