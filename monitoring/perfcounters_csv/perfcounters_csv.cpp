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
std::string one_hot_cpu_vector;
std::map<std::string, long long> counterSumOverThreads;
unsigned long long totalInstructions;

const int NUM_CPU_STATES = 10;
typedef struct CPUData
{
    std::string cpu;
    size_t times[NUM_CPU_STATES];
} CPUData;

class PerfManager
{
public:
    PerfManager(std::string benchmark, int cpu, std::vector<std::string> counterNames, unsigned int epochMs) : benchmark(benchmark), cpu(cpu), counterNames(counterNames), epoch(epochMs) {}
    void run()
    {
        // first wait for benchmark to start
        int pid = findBenchmark(benchmark, cpu);
        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;
        readStatsCPU(lastCPUData);

        observe(
            pid,
            true,
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
            if(name.find("CPU_CYCLES") != std::string::npos){
                //std::cout << "setting up CPU_CYCLES for " << name[0] - '0' << std::endl;
                std::shared_ptr<PerfCounter> counter = std::make_shared<PerfCounter>(-1, name[0] - '0', "CPU_CYCLES");
                counter->setUp();
                threadPerfCounters.push_back(counter);
            } else {
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
    bool periodic()
    {
        bool retVal = true;
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
        csv_file << std::to_string(getCurrentTemperature()) << ",";
        csv_file << one_hot_cpu_vector << ",";
        csv_file << getCurrentUtilization();


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
                if(counter->getName() == "INST_RETIRED")
                {
                    totalInstructions += (value - lastValue);
                    if(totalInstructions >= 10000000000)
                    {
                        retVal = false;
                    }
                }
            }
            lastValues.at(i) = value;
        }

        // set csv vals and reset counter
        for(const std::string& name : counterNames)
        {
            if(name.find("CPU_CYCLES") != std::string::npos)
            {
                csv_file << counterSumOverThreads[std::to_string(name[0] - '0') + "CPU_CYCLES"] << ",";
                counterSumOverThreads[std::to_string(name[0] - '0') + "CPU_CYCLES"] = 0;
            } else {
                csv_file << counterSumOverThreads[std::to_string(cpu) + name] << ",";
                counterSumOverThreads[std::to_string(cpu) + name] = 0;
            }
            //std::cout << std::to_string(cpu) + name << " : " << counterSumOverThreads[std::to_string(cpu) + name] << std::endl;
        }
        //std::cout << "totalInstructions: " << std::to_string(totalInstructions) << std::endl;
        csv_file << "\n";
        //std::cout << "-------------------------------------------------" << std::endl;
        return retVal;
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
        // 1st approach
        int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);

        char t[128];
        size_t r = pread(fd, t, 128, 0);
        if (r <= 0)
        {
            std::cout << "read weird number of bytes: " << r << std::endl;
            exit(1);
        }
        return atoi(t);
        /*
        // 2nd approach
        char temp[4];
        memcpy(temp, (const void*)0xf7030700, 4);
        int t = atoi(temp);

        std::cout << std::to_string(t) << std::endl;
        return t;
        */
    }

    void readStatsCPU(std::vector<CPUData> & entries)
    {
        std::ifstream fileStat("/proc/stat");

        std::string line;

        const std::string STR_CPU("cpu");
        const std::size_t LEN_STR_CPU = STR_CPU.size();

        while(std::getline(fileStat, line))
        {
            // cpu stats line found
            if(!line.compare(0, LEN_STR_CPU, STR_CPU))
            {
                std::istringstream ss(line);

                // store entry
                entries.emplace_back(CPUData());
                CPUData & entry = entries.back();

                // read cpu label
                ss >> entry.cpu;

                if(entry.cpu.size() > LEN_STR_CPU)
                    entry.cpu.erase(0, LEN_STR_CPU);
                else
                {
                    entries.pop_back();
                    continue;
                    //entry.cpu = "total";
                }

                // read times
                for(int i = 0; i < NUM_CPU_STATES; ++i)
                    ss >> entry.times[i];
            }
        }
    }

    std::string getCurrentUtilization()
    {
        std::string utilvector = "";
        std::vector<CPUData> newCPUData;
        readStatsCPU(newCPUData);
        const size_t num_entries = newCPUData.size();
        for(size_t i = 0; i < num_entries; i++)
        {
            const CPUData & e1 = lastCPUData[i];
            const CPUData & e2 = newCPUData[i];
            size_t last_idle = e1.times[3] + e1.times[4];
            size_t last_active = e1.times[0] + e1.times[1] + e1.times[2] + e1.times[5] + 
            e1.times[6] + e1.times[7] + e1.times[8] + e1.times[9];

            size_t new_idle = e2.times[3] + e2.times[4];
            size_t new_active = e2.times[0] + e2.times[1] + e2.times[2] + e2.times[5] + 
            e2.times[6] + e2.times[7] + e2.times[8] + e2.times[9];

            const float active_time = static_cast<float>(new_active - last_active);
            const float idle_time = static_cast<float>(new_idle - last_idle);
            const float total_time = active_time + idle_time;
            float cpu_load = (active_time/total_time);
            utilvector += std::to_string(cpu_load) + ",";
        }
        lastCPUData = newCPUData;
        return utilvector;
    }

    std::string benchmark;
    int cpu;
    std::vector<std::string> counterNames;
    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
    std::vector<std::shared_ptr<PerfCounter>> threadPerfCounters;
    std::vector<long long> lastValues;
    std::vector<CPUData> lastCPUData;
};

int main(int argc, char **argv)
{
    // ./percounters_csv <cpufreq> <benchmark> <cpu> <counter1> <counter2> ... <counterN>
    if (argc < 7)
    {
        //std::cerr << "first argument must be benchmark name or '?'" << std::endl;
        std::cerr << "call must be like ./perfcounters_csv <cpufreq_little> <cpufreq_big> <benchmark> <cpu> <scenario> <counter1> <counter2> ... <counterN>";
        return 1;
    }

    std::string cpufreq_little = argv[1];
    std::string cpufreq_big = argv[2];
    std::string benchmark = argv[3];
    int cpu = atoi(argv[4]);
    switch(cpu){
        case 1: 
            cpu = 0;
            one_hot_cpu_vector = "1,0,0,0,0,0,0,0";
            break;
        case 2: 
            cpu = 1;
            one_hot_cpu_vector = "0,1,0,0,0,0,0,0";
            break;
        case 4: 
            cpu = 2;
            one_hot_cpu_vector = "0,0,1,0,0,0,0,0";
            break;
        case 8: 
            cpu = 3;
            one_hot_cpu_vector = "0,0,0,1,0,0,0,0";
            break;
        case 10: 
            cpu = 4;
            one_hot_cpu_vector = "0,0,0,0,1,0,0,0";
            break;
        case 20: 
            cpu = 5;
            one_hot_cpu_vector = "0,0,0,0,0,1,0,0";
            break;
        case 40: 
            cpu = 6;
            one_hot_cpu_vector = "0,0,0,0,0,0,1,0";
            break;
        case 80: 
            cpu = 7;
            one_hot_cpu_vector = "0,0,0,0,0,0,0,1";
            break;
        default: std::cout << "error with cpu arg";
    }
    std::string scenario_file = argv[5];
    std::string scenario = std::regex_replace(scenario_file, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));


    std::string filename = "scenario" + scenario + "_benchmark-" + benchmark + "_core" + std::to_string(cpu) + "_frequencyl" + cpufreq_little + "_frequencyH" + cpufreq_big + ".csv";
    csv_file.open(filename);
    csv_file << "time,temperature,";
    csv_file << "curr_cpu0,curr_cpu1,curr_cpu2,curr_cpu3,curr_cpu4,curr_cpu5,curr_cpu6,curr_cpu7,";
    csv_file << "core_util0,core_util1,core_util2,core_util3,core_util4,core_util5,core_util6,core_util7,";
    std::vector<std::string> counterNames;
    for (int i = 6; i < argc; i++)
    {
    	//std::cout << "measuring " << argv[i] << std::endl;
        if(strcmp("CPU_CYCLES", argv[i]) == 0)
        {
            for(int j = 0; j < 8; j++)
            {
                counterNames.push_back(std::to_string(j) + argv[i]);
                csv_file << "cpu" << std::to_string(j) << argv[i] << ",";
                counterSumOverThreads[std::to_string(j) + argv[i]] = 0;
            }
        }
        else {
            counterNames.push_back(argv[i]);
            csv_file << argv[i] << ",";
            counterSumOverThreads[std::to_string(cpu) + argv[i]] = 0;
        }
    }
    csv_file << '\n';

    PerfManager m(benchmark, cpu, counterNames, 20);
    m.run();
    csv_file.close();

    //std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
