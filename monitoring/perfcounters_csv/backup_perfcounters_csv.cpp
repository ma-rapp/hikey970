
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <fstream>
#include <map>
#include <fcntl.h>

#include "console.h"
#include "perf.h"
#include "processes.h"


std::ofstream csv_file;
std::map<std::string, long long> counterSumOverThreads;
int startTime;
int cpus[4] { 4, 5, 6, 7 };

class PerfManager
{
public:
    PerfManager(std::string benchmark, std::vector<std::string> counterNames, unsigned int epochMs) : benchmark(benchmark), counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        startTime = currentDateTimeMilliseconds();

        // first wait for benchmark to start
        int pid = findBenchmark(benchmark);
        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        observe(
            pid,
            std::bind(&PerfManager::threadStarted, this, std::placeholders::_1),
            std::bind(&PerfManager::threadEnded, this, std::placeholders::_1),
            std::bind(&PerfManager::periodic, this),
            0 // waitPeriod
        );

        std::string nowText = std::to_string(currentDateTimeMilliseconds() - startTime);
        std::cout << nowText << "----------------------------" << std::endl;
        std::cout << nowText << "total values" << std::endl;
        for (const std::shared_ptr<PerfCounter>& counter : threadPerfCounters)
        {
            std::cout << nowText << " cpu " << counter->getCPUId() << " " << counter->getName() << ": " << counter->getCounterValue() << std::endl;
        }
    }

private:
    void threadStarted(int tid)
    {
        for (const std::string& name : counterNames)
        {
            for(int cpu : cpus){
                std::shared_ptr<PerfCounter> counter = std::make_shared<PerfCounter>(tid, cpu, name);
                counter->setUp();
                threadPerfCounters.push_back(counter);
            }
        }
    }

    void threadEnded(int tid)
    {
        for (const std::shared_ptr<PerfCounter>& counter : threadPerfCounters)
        {
            if (counter->getThreadId() == tid)
            {
                counter->tearDown();
            }
        }
    }
    void periodic()
    {
        std::string nowText = std::to_string(currentDateTimeMilliseconds() - startTime);

        // wait for 1 second
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::chrono::milliseconds untilNextUpdate = nextUpdate - now;
        nextUpdate += epoch;
        if (untilNextUpdate.count() > 0)
        {
            std::cout << nowText << " sleep for " << untilNextUpdate.count() << " ms" << std::endl;
            usleep(untilNextUpdate.count() * 1000);
        }
        else
        {
            std::cout << nowText << "late" << std::endl;
        }
        // time, temperature,
        csv_file << nowText << ",";
        csv_file << std::to_string(getCurrentTemperature()) << ",";

        // cpu4frequency,cpu5frequency,cpu6frequency,cpu7frequency,
        for(int cpu : cpus)
        {
            csv_file << std::to_string(getCurrentFrequency(cpu)) << ",";
        }

        for (unsigned int i = 0; i < threadPerfCounters.size(); i++)
        {
            const std::shared_ptr<PerfCounter> counter = threadPerfCounters.at(i);
            long long lastValue = 0;
            if (lastValues.size() <= i)
            {
                lastValues.push_back(lastValue);
            }
            else
            {
                lastValue = lastValues.at(i);
            }
            
            long long value = counter->getCounterValue();
            if (counter->isActive())
            {
                //std::cout << nowText << " cpu(" << counter->getCPUId()  << ") tid(" << counter->getThreadId() << ") " << counter->getName() << ": " << (value - lastValue) << std::endl;
                counterSumOverThreads[std::to_string(counter->getCPUId()) + counter->getName()] += (value - lastValue);
            }
            lastValues.at(i) = value;
        }

        // set csv vals and reset counter
        for(const std::string& name : counterNames)
        {
            for(int cpu : cpus)
            {
                std::cout << std::to_string(cpu) + name << " : " << counterSumOverThreads[std::to_string(cpu) + name] << std::endl;
                csv_file << counterSumOverThreads[std::to_string(cpu) + name] << ",";
                counterSumOverThreads[std::to_string(cpu) + name] = 0;
            }
        }
        csv_file << "\n";
        std::cout << "-------------------------------------------------" << std::endl;
        /*
        for (unsigned int i = 0; i < threadPerfCounters.size(); i++)
        {
            const std::shared_ptr<PerfCounter> counter = threadPerfCounters.at(i);
            if(counterSumOverThreads[std::to_string(counter->getCPUId()) + counter->getName()] == 0) continue;
            csv_file << counterSumOverThreads[std::to_string(counter->getCPUId()) + counter->getName()] << ",";
            counterSumOverThreads[std::to_string(counter->getCPUId()) + counter->getName()] = 0;
        }
        */
    }

    int getCurrentFrequency(int cpu_num)
    {
        std::string path = std::string("/sys/devices/system/cpu/cpu") + std::to_string(cpu_num) + std::string("/cpufreq/cpuinfo_cur_freq");
        int fd = open(path.c_str(), O_RDONLY);

        char freq[128];
        size_t r = pread(fd, freq, 128, 0);
        if (r <= 0)
        {
            std::cout << "read weird number of bytes: " << r << std::endl;
            exit(1);
        }

        return atoi(freq);
    }

    int getCurrentTemperature()
    {
        int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);

        char t[128];
        size_t r = pread(fd, t, 128, 0);
        if (r <= 0)
        {
            std::cout << "read weird number of bytes: " << r << std::endl;
            exit(1);
        }
        return atoi(t);
    }

    std::string benchmark;
    std::vector<std::string> counterNames;
    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
    std::vector<std::shared_ptr<PerfCounter>> threadPerfCounters;
    std::vector<long long> lastValues;
};

int main(int argc, char **argv)
{
    // ./percounters_csv <cpufreq> <benchmark> <counter1> <counter2> ... <counterN>
    if (argc < 3)
    {
        //std::cerr << "first argument must be benchmark name or '?'" << std::endl;
        std::cerr << "call must be like ./perfcounters_csv <cpufreq> <benchmark> <counter1> <counter2> ... <counterN>";
        return 1;
    }

    std::string cpufreq = argv[1];
    std::string benchmark = argv[2];

    csv_file.open("counterdata_" + cpufreq + ".csv");
    csv_file << "time,temperature,";

    for(int cpu : cpus)
    {
        csv_file << "cpu" << cpu << "frequency,";
    }

    std::vector<std::string> counterNames;
    for (int i = 3; i < argc; i++)
    {
        counterNames.push_back(argv[i]);
        // 1 counter for every core
        for(int cpu : cpus){
            csv_file << "cpu" << cpu << argv[i] << ",";
            counterSumOverThreads[std::to_string(cpu) + argv[i]] = 0;
        }
    }

    // time,temperature,cpu4frequency,cpu5frequency,cpu6frequency,cpu7frequency,cpu4INSTR_RETIRED,cpu5INSTR_RETIRED,cpu6INSTR_RETIRED,cpu7INSTR_RETIRED,cpu4CPU_CYCLES,cpu5CPU_CYCLES,cpu6CPU_CYCLES,cpu7CPU_CYCLES,

    csv_file << '\n';

    PerfManager m(benchmark, counterNames, 1000);
    m.run();
    csv_file.close();

    std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
