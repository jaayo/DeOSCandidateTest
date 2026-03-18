
#pragma once
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>

namespace sink_callback {

class ReentrantSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
  void sink_it_(const spdlog::details::log_msg&) override {
    // Fix: Removed the reentrant logging to avoid infinite recursion
  }
  void flush_() override {}
};

inline void run(std::atomic<bool>& stop, bool stress) {
  if (!stress) return;
  auto sink = std::make_shared<ReentrantSink>();
  auto logger = std::make_shared<spdlog::logger>("reentrant", sink);
  while (!stop.load()) {
    logger->warn("trigger");
  }
}
}

/*

Problem: ReentrantSink called spdlog::info() from within its own sink_it_ method which would create stack overflow due to infinite recursion.

The Why: This creates an infinite recursion loop.
          It is intermittent because it is guarded behind the --stress flag and only occurs when that specific logger is triggered.

The Fix: Removing the logging call from within the sink breaks the circular dependency. Sinks should handle data output (to files, console, etc.) without re-triggering the logging framework.

*/