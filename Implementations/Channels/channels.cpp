#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

const int TOTAL_NUMBERS = 10000;

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
    if(queue.empty())
      return false;
    if(wait)
      cv.wait(lock, [&](){ return closed || !queue.empty(); });
    out = queue.front();
    queue.pop_front();
    return true;
  }
};

int main() {
  auto begin = std::chrono::steady_clock::now();
  channel<int> channel;

  auto adder = std::thread ([&channel] () mutable {
    for (auto i = 0; i < TOTAL_NUMBERS; ++i) {
      channel.put(i);
    }
    channel.close();
  });

  auto consumer = std::thread ([&channel] () mutable {
    int i;
    while(channel.get(i));
  });

  adder.join();
  consumer.join();

  auto end = std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "µs" << std::endl;
}
