#pragma once

#include <functional>
#include <string>
#include <vector>

std::string getProcessName(int pid);
std::vector<int> getThreads(int pid);
int findBenchmark(std::string name);
bool processRunning(int pid);
void observe(int pid, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded);
void observe(int pid, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded, std::function<void(void)> periodicCallback, long waitPeriod=100*1000);
void observe(int pid, bool instruction_stoper, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded, std::function<bool(void)> periodicCallback, long waitPeriod=100*1000);
