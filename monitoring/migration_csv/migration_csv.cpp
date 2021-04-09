#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <fstream>
#include <map>
#include <fcntl.h>
#include <sstream>
#include <regex>
#include <iomanip>

#include "console.h"
#include "perf.h"
#include "processes.h"


std::ofstream csv_file;
std::map<std::string, long long> counterSumOverThreads;

class PerfManager
{
public:
    PerfManager(std::string benchmark, std::vector<std::string> counterNames, unsigned int epochMs) : benchmark(benchmark), counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        // first wait for benchmark to start
        int pid = findBenchmark(benchmark, -1);
        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        observe(
            pid,
            std::bind(&PerfManager::threadStarted, this, std::placeholders::_1),
            std::bind(&PerfManager::threadEnded, this, std::placeholders::_1),
            std::bind(&PerfManager::periodic, this),
            0 // waitPeriod
        );

        //std::string nowText = std::to_string(currentDateTimeMilliseconds());
        //std::cout << nowText << "----------------------------" << std::endl;
        //std::cout << nowText << "total values" << std::endl;
        //for (const std::shared_ptr<PerfCounter>& counter : threadPerfCounters)
        //{
            //std::cout << nowText << "tid " << counter->getThreadId() << " " << counter->getName() << ": " << counter->getCounterValue() << std::endl;
        //}
    }

private:
    void threadStarted(int tid)
    {
        for (const std::string& name : counterNames)
        {
            std::shared_ptr<PerfCounter> counter = std::make_shared<PerfCounter>(tid, -1, name);
            counter->setUp();
            threadPerfCounters.push_back(counter);
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
        std::string nowText = std::to_string(currentDateTimeMilliseconds());

        // wait for 1 second
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::chrono::milliseconds untilNextUpdate = nextUpdate - now;
        nextUpdate += epoch;
        if (untilNextUpdate.count() > 0)
        {
            //std::cout << nowText << " sleep for " << untilNextUpdate.count() << " ms" << std::endl;
            usleep(untilNextUpdate.count() * 1000);
        }
        else
        {
            std::cout << nowText << " late" << std::endl;
        }
        // time, temperature,
        csv_file << nowText << ",";


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
                counterSumOverThreads[counter->getName()] += (value - lastValue);
            }
            lastValues.at(i) = value;
        }

        // set csv vals and reset counter
        for(const std::string& name : counterNames)
        {
            //std::cout << std::to_string(cpu) + name << " : " << counterSumOverThreads[std::to_string(cpu) + name] << std::endl;
            csv_file << counterSumOverThreads[name] << ",";
            counterSumOverThreads[name] = 0;
        }
        //std::cout << "totalInstructions: " << std::to_string(totalInstructions) << std::endl;
        csv_file << "\n";
        //std::cout << "-------------------------------------------------" << std::endl;
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
    // ./percounters_csv <cpufreq> <benchmark> <cpu> <counter1> <counter2> ... <counterN>
    if (argc < 4)
    {
        //std::cerr << "first argument must be benchmark name or '?'" << std::endl;
        std::cerr << "call must be like ./perfcounters_csv <benchmark> <cpu> <counter1> <counter2> ... <counterN>";
        return 1;
    }

    std::string benchmark = argv[1];
    std::string cpu = argv[2];


    std::string filename = "migration_overhead" + cpu + "_test.csv";
    csv_file.open(filename);
    csv_file << "time,";
    std::vector<std::string> counterNames;
    for (int i = 3; i < argc; i++)
    {
    	//std::cout << "measuring " << argv[i] << std::endl;
        counterNames.push_back(argv[i]);
        csv_file << argv[i] << ",";
        counterSumOverThreads[argv[i]] = 0;
    }
    csv_file << '\n';

    PerfManager m(benchmark, counterNames, 20);
    m.run();
    csv_file.close();

    //std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
