
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

namespace string_uaf {

struct DeferredLog {

  //FIX: Store String instead of pointer 
  std::string val;

  void set() {
    val = "fixed:" + std::to_string(std::rand());
  }

  void emit() const {
    spdlog::info("deferred={}", val);
  }
};

inline void run(std::atomic<bool>& stop, bool stress) {
  while (!stop.load()) {
    DeferredLog d;
    d.set();
    std::this_thread::sleep_for(std::chrono::microseconds(stress ? 50 : 500));
    d.emit();
    std::this_thread::sleep_for(std::chrono::milliseconds(stress ? 1 : 10));
  }
}
}

/*

Problem: The DeferredLog struct uses a pointer to a string that is created in the set() method. When emit() is called, it tries to use that string. 
        If the DeferredLog object is destroyed (like when it's a local variable in a loop), the pointer becomes invalid, leading to crashes or unpredictable behavior.

The why: The local std::string would be destroyed when it goes out of scope at the end of the function. 
        It is intermittent because the memory might not be immediately overwritten by other threads, 
        allowing the program to possibly print the correct string before crashing or printing garbage.

The fix: By storing the string instead of the pointer, the struct takes ownership of the data. 
        The data will live as long as the DeferredLog instance exists, making it safe to access during the emit() call.

*/

