
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>

#include "console.h"
#include "perf.h"
#include "processes.h"

class CPUPerfManager
{
public:
    CPUPerfManager(int cpu, std::vector<std::string> counterNames, unsigned int epochMs) : cpu(cpu), counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        std::cout << currentDateTime() << "locked to CPU " << cpu << std::endl;

        for (const std::string& name : counterNames)
        {
            int tid = -1; // no thread
            std::shared_ptr<PerfCounter> counter = std::make_shared<PerfCounter>(tid, cpu, name);
            counter->setUp();
            cpuPerfCounters.push_back(counter);
        }

        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        while (true)
        {
            periodic();
        }

        std::string nowText = currentDateTime();
        std::cout << nowText << "----------------------------" << std::endl;
        std::cout << nowText << "total values" << std::endl;
        for (const std::shared_ptr<PerfCounter>& counter : cpuPerfCounters)
        {
            std::cout << nowText << "cpu " << counter->getCPUId() << " " << counter->getName() << ": " << counter->getCounterValue() << std::endl;
        }
    }

private:
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
        
        for (unsigned int i = 0; i < cpuPerfCounters.size(); i++)
        {
            const std::shared_ptr<PerfCounter> counter = cpuPerfCounters.at(i);
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
                std::cout << nowText << "cpu " << counter->getCPUId() << " " << counter->getName() << ": " << (value - lastValue) << std::endl;
            }
            lastValues.at(i) = value;
        }
    }

    int cpu;
    std::vector<std::string> counterNames;
    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
    std::vector<std::shared_ptr<PerfCounter>> cpuPerfCounters;
    std::vector<long long> lastValues;
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "first argument must be CPU id" << std::endl;
        return 1;
    }

    int cpu = atoi(argv[1]);

    std::vector<std::string> counterNames;
    for (int i = 2; i < argc; i++)
    {
        counterNames.push_back(argv[i]);
    }

    CPUPerfManager m(cpu, counterNames, 1000);
    m.run();

    std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
