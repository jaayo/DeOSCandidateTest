
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>

namespace shutdown {
inline void run(std::atomic<bool>& stop, bool stress) {
  spdlog::logger* raw = spdlog::default_logger_raw();

  // Launch thread
  std::thread t([&] {
    while (!stop.load()) raw->info("shutdown now");
  });

  if (stress) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  stop.store(true);

  // Fix: Wait for the thread to actually finish
  if (t.joinable()) {
    t.join();
  }

  // Clean up global resources
  if (stress) {
    spdlog::shutdown();
  }
}
}

/*

Problem: The code called spdlog::shutdown() while a background thread was still actively attempting to use the default logger.

The Why: spdlog::shutdown() destroys all loggers and cleans up resources. It is intermittent because it depends on whether the background thread hits its next loop iteration before or after the main thread finishes the shutdown process.

The Fix: By signaling the thread to stop and using t.join() to wait for its completion, the thread will exit completely before calling any global cleanup functions.

*/