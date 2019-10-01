
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <vector>

#include "console.h"
#include "perf.h"

class CCIPerfManager
{
public:
    CCIPerfManager(std::vector<std::string> counterNames, unsigned int epochMs) : counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        std::cout << currentDateTime() << "start" << std::endl;

        int nb = 0;
        for (const std::string& name : counterNames)
        {
            std::size_t pos = name.find(".");
            std::string interfaceName = name.substr(0, pos);
            std::string eventName = name.substr(pos + 1);
            std::shared_ptr<CCIPerfCounter> counter = std::make_shared<CCIPerfCounter>(nb++, interfaceName, eventName);
            counter->setUp();
            cciPerfCounters.push_back(counter);
        }

        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        while (true)
        {
            periodic();
        }

        std::string nowText = currentDateTime();
        std::cout << nowText << "----------------------------" << std::endl;
        std::cout << nowText << "total values" << std::endl;
        for (const std::shared_ptr<CCIPerfCounter>& counter : cciPerfCounters)
        {
            std::cout << nowText << counter->getInterfaceName() << "." << counter->getEventName() << ": " << counter->getCounterValue() << std::endl;
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
        
        for (unsigned int i = 0; i < cciPerfCounters.size(); i++)
        {
            const std::shared_ptr<CCIPerfCounter> counter = cciPerfCounters.at(i);
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
            long long diff = value - lastValue;
            if (diff < 0)
            {
                // overflow
                diff += 1L << 32;
            }
            std::cout << nowText << counter->getInterfaceName() << "." << counter->getEventName() << ": " << diff << std::endl;
            lastValues.at(i) = value;
        }
    }

    std::vector<std::string> counterNames;
    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
    std::vector<std::shared_ptr<CCIPerfCounter>> cciPerfCounters;
    std::vector<long long> lastValues;
};

int main(int argc, char **argv)
{
    std::vector<std::string> counterNames;
    for (int i = 1; i < argc; i++)
    {
        counterNames.push_back(argv[i]);
    }

    CCIPerfManager m(counterNames, 1000);
    m.run();

    std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
