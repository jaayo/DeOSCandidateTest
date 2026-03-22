
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>
#include <string>

namespace level {
// Fix: Atomic variable
inline std::atomic<int> g_level{0};

class Level5
{
public:
  Level5(std::string ss): s(std::move(ss)) {};
  ~Level5() = default; // Fix: default destructor

  void print() {
    spdlog::debug(s);
  }

private:
  std::string s;
};

inline void run(std::atomic<bool>& stop, bool) {
  std::thread w([&] {
    while (!stop.load()) g_level.fetch_add(1, std::memory_order_relaxed);
  });
  std::thread r([&] {
    while (!stop.load()) {
      int number = g_level.load(std::memory_order_relaxed);
      if (number & 1) spdlog::info("odd " + std::to_string(number));
      else spdlog::info("even " + std::to_string(number));
    }
  });
  w.join();
  r.join();
}
}
/*

The Problem: The variable g_level was accessed by 2 different threads without synchronization. The Level5 destructor attempted to delete the address of a stack-allocated member variable.

The Why: Multiple threads reading/writing the same memory without a mutex or atomic wrapper is "Undefined Behavior. 
        The string s is a member variable, calling delete on its adress causes memory corruption. 
        It is intermittent because it only happens if the string s happens to be "j". Since its initialized as "level 5 logs" in main.cpp, it doesn't crash yet.

The Fix: By changing g_level to an atomic variable, we ensure that all threads see a consistent view of its value. 
        Removing the manual delete allows the std::string destructor to handle its own memory automatically

*/