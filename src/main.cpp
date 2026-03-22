#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <thread>

#include "include/string_uaf.h"
#include "include/sink_callback.h"
#include "include/shutdown.h"
#include "include/timebase.h"
#include "include/level.h"

// Structure to hold command-line arguments
struct Args {
  int seconds = 3;
  bool stress = false; 
};

// Function to parse command-line arguments
static Args parse_args(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--seconds") == 0 && i + 1 < argc) {
      a.seconds = std::atoi(argv[++i]); 
    } else if (std::strcmp(argv[i], "--stress") == 0) {
      a.stress = true; 
    }
  }
  return a;
}

int main(int argc, char** argv) {
  auto args = parse_args(argc, argv);

  spdlog::info("Program started with seconds={} and stress={}", args.seconds, args.stress); // Debugging log

  // Set up asynchronous logging to a file
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("output.log", true);
  auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
  auto logger = std::make_shared<spdlog::async_logger>(
      "lab",
      spdlog::sinks_init_list{file_sink},
      tp,
      spdlog::async_overflow_policy::overrun_oldest);

  // Configure the logger as the default logger
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::info); // Set logging level to info

  // Atomic flag to signal threads to stop
  std::atomic<bool> stop{false};

  // Create a Level5 object for managing specific logs
  level::Level5 level5("level 5 logs");

  // Launch threads to execute various tasks
  std::thread t1([&] { string_uaf::run(stop, args.stress); }); // Thread for string_uaf
  std::thread t2([&] { timebase::run(stop, args.stress); });   // Thread for timebase
  std::thread t3([&] { level::run(stop, args.stress); });      // Thread for level
  std::thread t4([&] { shutdown::run(stop, args.stress); });   // Thread for shutdown
  std::thread t5([&] { sink_callback::run(stop, args.stress); }); // Thread for sink_callback

  spdlog::info("Launching threads"); // Debugging log

  // Run a heartbeat loop for the specified duration
  auto end = std::chrono::steady_clock::now() + std::chrono::seconds(args.seconds);
  spdlog::info("Entering heartbeat loop"); // Debugging log
  while (std::chrono::steady_clock::now() < end) {
    // spdlog::info("heartbeat"); 
    spdlog::info("heartbeat, stop value: {}", stop.load()); // Debugging log
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
    level5.print(); // Print level 5 logs
  }
  spdlog::info("Exiting heartbeat loop"); // Debugging log

  // Signal threads to stop and wait for their completion
  stop.store(true);
  t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
  spdlog::info("All threads joined"); // Debugging log

  // Flush the async buffer to ensure output.log gets the last messages
  logger->flush();

  // Shutdown the logger and exit
  spdlog::shutdown();
  return 0;
}


/*
The Problem: The program terminated and shut down the logging system without ensuring the asynchronous worker thread had finished writing the buffered logs.
 
The Why: Because spdlog::async_logger uses a background thread and a queue, messages can be lost if the program exits while the queue is still full.

The Fix: Adding logger->flush() before spdlog::shutdown() forces the async worker to empty the queue to the file. Combined with joining all worker threads before shutdown, this ensures no data loss and no use-after-free errors at program exit.
*/