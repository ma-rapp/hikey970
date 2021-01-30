
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <fstream>

#include "console.h"
#include "perf.h"
#include "processes.h"


std::ofstream csv_file;

class PerfManager
{
public:
    PerfManager(int pid, std::vector<std::string> counterNames, unsigned int epochMs) : pid(pid), counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;
        observe(
            pid,
            std::bind(&PerfManager::threadStarted, this, std::placeholders::_1),
            std::bind(&PerfManager::threadEnded, this, std::placeholders::_1),
            std::bind(&PerfManager::periodic, this),
            0 // waitPeriod
        );

        std::string nowText = currentDateTime();
        std::cout << nowText << "----------------------------" << std::endl;
        std::cout << nowText << "total values" << std::endl;
        for (const std::shared_ptr<PerfCounter>& counter : threadPerfCounters)
        {
            std::cout << nowText << "tid " << counter->getThreadId() << " " << counter->getName() << ": " << counter->getCounterValue() << std::endl;
        }
    }

private:
    void threadStarted(int tid)
    {
        for (const std::string& name : counterNames)
        {
            int cpu = -1; // all CPUs
            std::shared_ptr<PerfCounter> counter = std::make_shared<PerfCounter>(tid, cpu, name);
            counter->setUp();
            threadPerfCounters.push_back(counter);
        }
        for (const std::shared_ptr<PerfCounter>& counter : threadPerfCounters)
        {
            csv_file << counter->getName() << ',';
        }
        csv_file << '\n';
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
        std::string nowText = currentDateTime();

        // wait for 1 second
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::chrono::milliseconds untilNextUpdate = nextUpdate - now;
        nextUpdate += epoch;
        if (untilNextUpdate.count() > 0)
        {
            std::cout << nowText << "sleep for " << untilNextUpdate.count() << " ms" << std::endl;
            usleep(untilNextUpdate.count() * 1000);
        }
        else
        {
            std::cout << nowText << "late" << std::endl;
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
                std::cout << nowText << "tid " << counter->getThreadId() << " " << counter->getName() << ": " << (value - lastValue) << std::endl;
                csv_file << (value - lastValue) << ',';
            }
            lastValues.at(i) = value;
        }
        csv_file << '\n';
    }

    int pid;
    std::vector<std::string> counterNames;
    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
    std::vector<std::shared_ptr<PerfCounter>> threadPerfCounters;
    std::vector<long long> lastValues;
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "first argument must be benchmark name or '?'" << std::endl;
        return 1;
    }

    std::vector<std::string> counterNames;
    for (int i = 2; i < argc; i++)
    {
        counterNames.push_back(argv[i]);
    }

    std::string benchmark = argv[1];
    int pid = findBenchmark(benchmark);

    PerfManager m(pid, counterNames, 1000);
    csv_file.open("counterdata.csv");
    m.run();
    csv_file.close();

    std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
