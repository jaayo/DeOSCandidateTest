
Identified Correctness Issues

1. string_uaf.h: Use-After-Free 
The program was attempting to access data from a string that had already been destroyed.

2. shutdown.h: Destructive Race Condition 
The global logging environment was being deleted down while worker threads were still actively attempting to log. 

3. sink_callback.h: Recursive Logging Loop 
A sink was triggering the logger that called it, creating an infinite recursion.

4. timebase.h: Clock Type Difference 
Latency calculations were performed by subtracting a two incompatible clock types, resulting in nonsense data

5. level.h: Data Races & Stack Corruption 
Global integers were modified across threads without atomic protection, and a destructor was illegally attempting to manually delete stack-allocated memory.

6. main.cpp: Async Buffer Loss
The async logging queue was not being properly drained before the application exited.


For the full analysis of and fixed code of each each issue, see the comments under every file. 


Completed by: Jasper Ayotte-Veltman
